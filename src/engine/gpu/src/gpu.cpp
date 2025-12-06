#include "engine/memory.hpp"
#include <array>
#include <engine/vec.hpp>
// #include <cmath>
#include <engine/gltf.hpp>
#include <engine/gpu.hpp>
#include <glad/glad.h>
#include <iostream>

#define UNIFORM_NAME_SKIN "u_skin"
#define UNIFORM_NAME_SKIN_COUNT "u_skin_count"
#define UNIFORM_NAME_SKIN_START "u_skin_start"
#define UNIFORM_NAME_MODEL_MAT4 "u_model"
#define UNIFORM_NAME_VIEW_MAT4 "u_view"
#define UNIFORM_NAME_PROJECTION_MAT4 "u_projection"
#define UNIFORM_NAME_NORMAL_MAT3 "u_normal"
#define UNIFORM_NAME_MVP_MAT4 "u_mvp"
#define UNIFORM_NAME_MATERIAL_ALBEDO_TEX "u_tex_color"

#define POSE_TEXTURE_UNIT 0
#define COLOR_TEXTURE_UNIT 1

#define gl_check_error()                                                       \
    {                                                                          \
        GLuint error;                                                          \
        if (GL_NO_ERROR != (error = glGetError()))                             \
            throw engine::gpu::exception::base("GL error " +                   \
                                               std::to_string(error));         \
    }

#define gl_call(name, ...)                                                     \
    {                                                                          \
        GLuint error;                                                          \
        name(__VA_ARGS__);                                                     \
                                                                               \
        if (GL_NO_ERROR != (error = glGetError()))                             \
        {                                                                      \
            std::cerr << "GL error: " + std::to_string(error) + "\n";          \
            throw engine::gpu::exception::base(                                \
                "GL error " + std::to_string(error) +                          \
                " after calling function " #name);                             \
        }                                                                      \
    }

engine::gpu::asset::asset(const gltf::gltf &in)
{
    for (const gltf::texture &in_tex : in.textures)
        textures.emplace(in_tex.name, in_tex);

    for (const gltf::material &in_material : in.materials)
        if (!in_material.name.empty())
            materials.emplace(in_material.name,
                              engine::gpu::asset::material(*this, in_material));

    for (const gltf::skin &in_skin : in.skins)
        if (!in_skin.name.empty())
            armatures.emplace(in_skin.name, skel::armature(in_skin, in));

    for (const gltf::animation &in_animation : in.animations)
        if (!in_animation.name.empty())
            animations.emplace(in_animation.name,
                               skel::animation(in_animation, in));

    for (const gltf::mesh &in_mesh : in.meshes)
        if (!in_mesh.name.empty())
            meshes.emplace(in_mesh.name,
                           engine::gpu::asset::mesh(*this, in_mesh));

    for (const gltf::node &in_node : in.nodes)
        if (!in_node.name.empty() && in_node.mesh)
            objects.emplace(in_node.name, object(*this, in_node));
}

engine::gpu::asset::object::object(const asset &parent, const gltf::node &in)
    : mesh(parent.meshes.at(in.mesh->name))
{
    if (in.skin)
    {
        skin = &parent.armatures.at(in.skin->name);
        skin_name = in.skin->name;
    }
}

void engine::gpu::asset::object::draw(
    engine::gpu::shader::program &program) const
{
    mesh.draw(program);
}

engine::gpu::asset::asset(const std::string &path, gltf::gltf_cache &cache)
    : engine::gpu::asset::asset(*cache[path])
{
}

void engine::gpu::asset::draw(const std::string &mesh_name,
                              engine::gpu::shader::program &program) const
{
    auto it = objects.find(mesh_name);

    if (it == objects.end())
        return;

    it->second.draw(program);
}

engine::gpu::asset::material::material(const engine::gpu::asset &parent,
                                       const gltf::material &in)
{
#define lookup_tex(tname)                                                      \
    if (in.tname)                                                              \
    {                                                                          \
        auto it = parent.textures.find(in.tname->texture.name);                \
        if (it != parent.textures.end())                                       \
            tname = &it->second;                                               \
    }

    lookup_tex(normal_texture);
    lookup_tex(occlusion_texture);
    lookup_tex(emissive_texture);

#undef lookup_tex

    if (in.pbr_metallic_roughness)
    {
        const gltf::pbr_metallic_roughness &pbr = *in.pbr_metallic_roughness;

        if (pbr.metallic_roughness_texture.has_value())
        {
            const std::string &in_name =
                pbr.metallic_roughness_texture->texture.name;
            auto it = parent.textures.find(in_name);
            if (it != parent.textures.end())
                metallic_roughness_texture = &it->second;
        }

        if (pbr.base_color_texture.has_value())
        {
            const std::string &in_name = pbr.base_color_texture->texture.name;
            auto it = parent.textures.find(in_name);
            if (it != parent.textures.end())
                base_color_texture = &it->second;
        }
        base_color_factor = pbr.base_color_factor;
        metallic = pbr.metallic_factor;
        roughness = pbr.roughness_factor;
    }

    emissive_factor = in.emissive_factor;
    double_sided = in.double_sided;
    alpha_cutoff = in.alpha_cutoff;
}

void engine::gpu::asset::material::use(
    engine::gpu::shader::program &program) const
{
    if (base_color_texture)
        program.set_albedo_texture(*base_color_texture);
}

engine::gpu::asset::primitive::primitive(
    const gpu::asset::material &_material,
    const class gltf::mesh_primitive &input)
    : material(_material)
{
    gl_check_error();

    struct vertex_attribute
    {
        const gltf::accessor *accessor;
        uint32_t index;
        bool normalized;
        gltf::component_type component_type;
        gltf::attribute_type attribute_type;
    };

    std::array<vertex_attribute, 6> attributes = {
        {{.accessor = input.attributes.position,
          .index = 0,
          .normalized = false,
          .component_type = gltf::component_type::FLOAT,
          .attribute_type = gltf::attribute_type::VEC3},
         {.accessor = input.attributes.normal,
          .index = 1,
          .normalized = true,
          .component_type = gltf::component_type::SHORT,
          .attribute_type = gltf::attribute_type::VEC3},
         {.accessor = input.attributes.tangent,
          .index = 2,
          .normalized = true,
          .component_type = gltf::component_type::SHORT,
          .attribute_type = gltf::attribute_type::VEC4},
         {.accessor = input.attributes.texcoord_0,
          .index = 3,
          .normalized = true,
          .component_type = gltf::component_type::FLOAT,
          .attribute_type = gltf::attribute_type::VEC2},
         {.accessor = input.attributes.joints,
          .index = 4,
          .normalized = false,
          .component_type = gltf::component_type::UBYTE,
          .attribute_type = gltf::attribute_type::VEC4},
         {.accessor = input.attributes.weights,
          .index = 5,
          .normalized = true,
          .component_type = gltf::component_type::UBYTE,
          .attribute_type = gltf::attribute_type::VEC4}}};

    gl_call(glGenBuffers, 1, &vbo);
    gl_call(glGenVertexArrays, 1, &vao);
    gl_call(glBindVertexArray, vao);

    if (input.indices)
    {
        gl_call(glGenBuffers, 1, &ibo);
        gl_call(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, ibo);

        if (input.indices->count < std::numeric_limits<uint16_t>::max())
        {
            std::vector<uint16_t> indices = *input.indices;
            gl_call(glBufferData,
                    GL_ELEMENT_ARRAY_BUFFER,
                    indices.size() * sizeof(uint16_t),
                    indices.data(),
                    GL_STATIC_DRAW);
            short_indices = true;
        }
        else
        {
            std::vector<uint32_t> indices = *input.indices;
            gl_call(glBufferData,
                    GL_ELEMENT_ARRAY_BUFFER,
                    indices.size() * sizeof(uint32_t),
                    indices.data(),
                    GL_STATIC_DRAW);
            short_indices = false;
        }
        count = input.indices->count;
    }
    else
    {
        if (!input.attributes.position)
            throw engine::gpu::exception::base(
                "Cannot create GPU primitive without position accessor or "
                "indices");
        count = input.attributes.position->count;
    }

    gl_call(glBindBuffer, GL_ARRAY_BUFFER, vbo);

    std::vector<uint8_t> vertex_data;

    for (const vertex_attribute &attribute : attributes)
    {
        if (!attribute.accessor)
            continue;

        if (attribute.accessor->component_type == gltf::component_type::FLOAT ||
            attribute.normalized)
        {
            gl_call(glVertexAttribPointer,
                    attribute.index,
                    (GLint)attribute.attribute_type,
                    (GLenum)attribute.component_type,
                    attribute.normalized,
                    0,
                    (void *)vertex_data.size());
            gl_call(glEnableVertexAttribArray, attribute.index);
        }
        else
        {
            gl_call(glVertexAttribIPointer,
                    attribute.index,
                    (GLint)attribute.attribute_type,
                    (GLenum)attribute.component_type,
                    0,
                    (void *)vertex_data.size());
            gl_call(glEnableVertexAttribArray, attribute.index);
        }
        attribute.accessor->dump(vertex_data,
                                 attribute.component_type,
                                 attribute.attribute_type);
    }

    gl_call(glBindBuffer, GL_ARRAY_BUFFER, vbo);
    gl_call(glBufferData,
            GL_ARRAY_BUFFER,
            vertex_data.size(),
            vertex_data.data(),
            GL_STATIC_DRAW);

    gl_call(glBindVertexArray, 0);
    gl_call(glBindBuffer, GL_ARRAY_BUFFER, 0);

    for (const vec::fvec3 &position :
         (std::vector<vec::fvec3>)(*input.attributes.position))
    {
        float length = vec::length(position);
        if (radius < length)
            radius = length;
    }
}

engine::gpu::asset::primitive::primitive(primitive &&other) noexcept
    : vao(other.vao), vbo(other.vbo), ibo(other.ibo), count(other.count),
      short_indices(other.short_indices), material(other.material)
{
    other.vao = 0;
    other.vbo = 0;
    other.ibo = 0;
    other.count = 0;
}

engine::gpu::asset::primitive::~primitive()
{
    if (ibo)
        glDeleteBuffers(1, &ibo);
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (vao)
        glDeleteVertexArrays(1, &vao);
}

void engine::gpu::asset::primitive::bind() const
{
    gl_call(glBindVertexArray, vao);
    gl_call(glBindBuffer, GL_ARRAY_BUFFER, vbo);
    if (ibo)
        gl_call(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void engine::gpu::asset::primitive::draw() const
{
    if (ibo)
    {
        if (short_indices)
        {
            gl_call(glDrawElements,
                    GL_TRIANGLES,
                    count,
                    GL_UNSIGNED_SHORT,
                    nullptr);
        }
        else
        {
            gl_call(glDrawElements,
                    GL_TRIANGLES,
                    count,
                    GL_UNSIGNED_INT,
                    nullptr);
        }
    }
    else
    {
        gl_call(glDrawArrays, GL_TRIANGLES, 0, count);
    }
}

engine::gpu::asset::mesh::mesh(const engine::gpu::asset &parent,
                               const class gltf::mesh &in_mesh)
    : radius(0)
{
    for (const gltf::mesh_primitive &in_primitive : in_mesh.primitives)
    {
        if (!in_primitive.material)
            continue;
        const std::string &in_name = in_primitive.material->name;
        if (in_name.empty())
            continue;
        auto it = parent.materials.find(in_name);
        if (it == parent.materials.end())
            continue;
        primitives.emplace_back(it->second, in_primitive);
    }
    for (const engine::gpu::asset::primitive &primitive : primitives)
        if (radius < primitive.radius)
            radius = primitive.radius;
}

engine::gpu::asset::mesh::mesh(mesh &&other) noexcept
{
    radius = other.radius;
    primitives = std::move(other.primitives);
}
void engine::gpu::asset::mesh::draw(
    engine::gpu::shader::program &in_shader) const
{
    for (const engine::gpu::asset::primitive &prim : primitives)
    {
        prim.material.use(in_shader);
        prim.bind();
        prim.draw();
    }
}

engine::gpu::asset::texture::texture(const gltf::texture &texture)
{
    gl_check_error();

    gl_call(glGenTextures, 1, &id);
    if (!id)
        throw gpu::exception::base("Could not create texture");
    gl_call(glBindTexture, GL_TEXTURE_2D, id);

    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            (GLint)texture.sampler.min_filter);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            (GLint)texture.sampler.mag_filter);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            (GLint)texture.sampler.wrap_s);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            (GLint)texture.sampler.wrap_t);

    const engine::image::rgba32 &image = texture.source.contents;

    gl_call(glTexImage2D,
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            (GLsizei)image.width,
            (GLsizei)image.height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image.data());

    gl_call(glGenerateMipmap, GL_TEXTURE_2D);

    gl_call(glBindTexture, GL_TEXTURE_2D, 0);

    assert(glIsTexture(id));

    std::cerr << "Created texture id " << id << std::endl;
}

engine::gpu::asset::texture::texture(texture &&other) noexcept : id(other.id)
{
    other.id = 0;
}

// engine::gpu::asset::texture& engine::gpu::asset::texture::operator=(texture&&
// other) noexcept
// {
//     if (this != &other)
//     {
//         if (id)
//             glDeleteTextures(1, &id);
//         id = other.id;
//         other.id = 0;
//     }
//     return *this;
// }

engine::gpu::asset::texture::~texture()
{
    if (id)
        glDeleteTextures(1, &id);
}

void engine::gpu::asset::texture::bind(uint32_t unit) const
{
    gl_check_error();
    assert(glIsTexture(id));
    gl_call(glActiveTexture, GL_TEXTURE0 + (GLenum)unit);
    gl_call(glBindTexture, GL_TEXTURE_2D, id);
}

engine::gpu::target::target() : fbo(0), color(0), depth_stencil(0) {}

engine::gpu::target::target(uint32_t width, uint32_t height)
{
    gl_check_error();

    gl_call(glGenFramebuffers, 1, &fbo);
    gl_call(glBindFramebuffer, GL_FRAMEBUFFER, fbo);

    gl_call(glGenTextures, 1, &color);
    gl_call(glBindTexture, GL_TEXTURE_2D, color);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexImage2D,
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            (GLsizei)width,
            (GLsizei)height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr);
    gl_call(glFramebufferTexture2D,
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            color,
            0);

    gl_call(glGenTextures, 1, &depth_stencil);
    gl_call(glBindTexture, GL_TEXTURE_2D, depth_stencil);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexImage2D,
            GL_TEXTURE_2D,
            0,
            GL_DEPTH24_STENCIL8,
            (GLsizei)width,
            (GLsizei)height,
            0,
            GL_DEPTH_STENCIL,
            GL_UNSIGNED_INT_24_8,
            nullptr);
    gl_call(glFramebufferTexture2D,
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_TEXTURE_2D,
            depth_stencil,
            0);

    GLenum draw_buffers = GL_COLOR_ATTACHMENT0;
    gl_call(glDrawBuffers, 1, &draw_buffers);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        gl_call(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        throw engine::gpu::exception::base(
            "Failed to create render target: incomplete framebuffer");
    }

    gl_call(glBindTexture, GL_TEXTURE_2D, 0);
    gl_call(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

engine::gpu::target::~target()
{
    if (depth_stencil)
        glDeleteTextures(1, &depth_stencil);
    if (color)
        glDeleteTextures(1, &color);
    if (fbo)
        glDeleteFramebuffers(1, &fbo);
}

void engine::gpu::target::bind()
{
    gl_call(glBindFramebuffer, GL_FRAMEBUFFER, fbo);
}

engine::gpu::gbuffer::gbuffer(uint32_t width, uint32_t height)
{
#define POSITION_FORMAT GL_RGB32F
#define NORMAL_FORMAT GL_RGB16F
#define ALBEDO_SPECULAR_FORMAT GL_RGBA8
#define DEPTH_STENCIL_FORMAT GL_DEPTH24_STENCIL8

    gl_check_error();

    gl_call(glGenFramebuffers, 1, &fbo);
    gl_call(glBindFramebuffer, GL_FRAMEBUFFER, fbo);

    gl_call(glGenTextures, 1, &position);
    gl_call(glBindTexture, GL_TEXTURE_2D, position);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexImage2D,
            GL_TEXTURE_2D,
            0,
            POSITION_FORMAT,
            (GLsizei)width,
            (GLsizei)height,
            0,
            GL_RGB,
            GL_FLOAT,
            nullptr);
    gl_call(glFramebufferTexture2D,
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            position,
            0);

    gl_call(glGenTextures, 1, &normal);
    gl_call(glBindTexture, GL_TEXTURE_2D, normal);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexImage2D,
            GL_TEXTURE_2D,
            0,
            NORMAL_FORMAT,
            (GLsizei)width,
            (GLsizei)height,
            0,
            GL_RGB,
            GL_FLOAT,
            nullptr);
    gl_call(glFramebufferTexture2D,
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT1,
            GL_TEXTURE_2D,
            normal,
            0);

    gl_call(glGenTextures, 1, &albedo_specular);
    gl_call(glBindTexture, GL_TEXTURE_2D, albedo_specular);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexImage2D,
            GL_TEXTURE_2D,
            0,
            ALBEDO_SPECULAR_FORMAT,
            (GLsizei)width,
            (GLsizei)height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr);
    gl_call(glFramebufferTexture2D,
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT2,
            GL_TEXTURE_2D,
            albedo_specular,
            0);

    gl_call(glGenTextures, 1, &depth_stencil);
    gl_call(glBindTexture, GL_TEXTURE_2D, depth_stencil);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexImage2D,
            GL_TEXTURE_2D,
            0,
            DEPTH_STENCIL_FORMAT,
            (GLsizei)width,
            (GLsizei)height,
            0,
            GL_DEPTH_STENCIL,
            GL_UNSIGNED_INT_24_8,
            nullptr);
    gl_call(glFramebufferTexture2D,
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_TEXTURE_2D,
            depth_stencil,
            0);

    GLenum draw_buffers[3] = {GL_COLOR_ATTACHMENT0,
                              GL_COLOR_ATTACHMENT1,
                              GL_COLOR_ATTACHMENT2};
    gl_call(glDrawBuffers, 3, draw_buffers);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        gl_call(glBindFramebuffer, GL_FRAMEBUFFER, 0);
        throw engine::gpu::exception::base(
            "Failed to create gbuffer: incomplete framebuffer");
    }

    gl_call(glBindTexture, GL_TEXTURE_2D, 0);
    gl_call(glBindFramebuffer, GL_FRAMEBUFFER, 0);
}

engine::gpu::gbuffer::~gbuffer()
{
    if (depth_stencil)
        glDeleteTextures(1, &depth_stencil);
    if (albedo_specular)
        glDeleteTextures(1, &albedo_specular);
    if (normal)
        glDeleteTextures(1, &normal);
    if (position)
        glDeleteTextures(1, &position);
    if (fbo)
        glDeleteFramebuffers(1, &fbo);
}

void engine::gpu::gbuffer::bind()
{
    gl_call(glBindFramebuffer, GL_FRAMEBUFFER, fbo);
}

engine::gpu::shader::shader::shader(uint32_t type, std::string source)
{
    gl_check_error();

    id = glCreateShader(type);
    const char *source_cstr = source.c_str();
    gl_call(glShaderSource, id, 1, &source_cstr, nullptr);
    gl_call(glCompileShader, id);

    GLint compile_status = GL_FALSE;
    gl_call(glGetShaderiv, id, GL_COMPILE_STATUS, &compile_status);
    if (compile_status != GL_TRUE)
    {
        GLint log_length = 0;
        gl_call(glGetShaderiv, id, GL_INFO_LOG_LENGTH, &log_length);

        std::string log(log_length, '\0');
        gl_call(glGetShaderInfoLog, id, log_length, nullptr, log.data());

        glDeleteShader(id);
        std::string message = "Failed to compile shader: " + log;
        std::cerr << message << std::endl;
        throw engine::gpu::exception::base(message);
    }
}

engine::gpu::shader::shader::~shader()
{
    if (id)
        glDeleteShader(id);
}

engine::gpu::shader::vertex::vertex::vertex(const std::string &source)
    : shader(GL_VERTEX_SHADER, source)
{
}

engine::gpu::shader::vertex::vertex::vertex(
    const engine::memory::allocation &source)
    : engine::gpu::shader::vertex::vertex(
          std::string((const char *)source.data(), source.size()))
{
}

engine::gpu::shader::vertex::vertex::vertex(
    engine::filesystem::cache_binary &fs,
    const std::string &path)
    : engine::gpu::shader::vertex(*fs[path])
{
}

engine::gpu::shader::fragment::fragment::fragment(std::string source)
    : shader(GL_FRAGMENT_SHADER, source)
{
}

engine::gpu::shader::fragment::fragment::fragment(
    const engine::memory::allocation &source)
    : engine::gpu::shader::fragment::fragment(
          std::string((const char *)source.data(), source.size()))
{
}

engine::gpu::shader::fragment::fragment::fragment(
    engine::filesystem::cache_binary &fs,
    const std::string &path)
    : engine::gpu::shader::fragment(*fs[path])
{
}

engine::gpu::shader::program::program(const vertex *vertex_shader,
                                      const fragment *fragment_shader)
{
    gl_check_error();
    id = glCreateProgram();
    if (vertex_shader)
        gl_call(glAttachShader, id, *vertex_shader);
    if (fragment_shader)
        gl_call(glAttachShader, id, *fragment_shader);
    gl_call(glLinkProgram, id);
    GLint link_status = GL_FALSE;
    gl_call(glGetProgramiv, id, GL_LINK_STATUS, &link_status);
#define buf_size 512

    if (link_status != GL_TRUE)
    {
        GLint log_length = 0;
        gl_call(glGetProgramiv, id, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > buf_size)
            log_length = buf_size;
        GLchar log[buf_size];
        gl_call(glGetProgramInfoLog, id, log_length, nullptr, log);
        glDeleteProgram(id);
        throw engine::gpu::exception::base("Failed to link program: " +
                                           std::string(log, log_length));
    }

    if ((u_skin = glGetUniformLocation(id, UNIFORM_NAME_SKIN)) < 0)
        std::cerr << "No skin uniform\n";

    if ((u_skin_count = glGetUniformLocation(id, UNIFORM_NAME_SKIN_COUNT)) < 0)
        std::cerr << "No skin count uniform\n";

    if ((u_model = glGetUniformLocation(id, UNIFORM_NAME_MODEL_MAT4)) < 0)
        std::cerr << "No model uniform\n";

    if ((u_view = glGetUniformLocation(id, UNIFORM_NAME_VIEW_MAT4)) < 0)
        std::cerr << "No view matrix uniform\n";

    if ((u_normal = glGetUniformLocation(id, UNIFORM_NAME_NORMAL_MAT3)) < 0)
        std::cerr << "No normal matrix uniform\n";

    if ((u_projection =
             glGetUniformLocation(id, UNIFORM_NAME_PROJECTION_MAT4)) < 0)
        std::cerr << "No projection matrix uniform\n";

    if ((u_mvp = glGetUniformLocation(id, UNIFORM_NAME_MVP_MAT4)) < 0)
        std::cerr << "No model-view-projection matrix uniform\n";

    if ((u_albedo_tex =
             glGetUniformLocation(id, UNIFORM_NAME_MATERIAL_ALBEDO_TEX)) < 0)
        std::cerr << "No albedo texture uniform\n";
}

engine::gpu::shader::program::~program()
{
    if (id)
        glDeleteProgram(id);
}

void engine::gpu::shader::program::bind()
{
    gl_call(glUseProgram, id);
}

void engine::gpu::shader::program::set_skin(const engine::gpu::skin &skin)
{
    if (u_skin != -1)
        gl_call(glUniform1i, u_skin, POSE_TEXTURE_UNIT);

    skin_bone_count = skin.bone_count;
}

void engine::gpu::shader::program::set_skin_slice(
    const skel::pose::slice &slice)
{
    if (slice.begin + slice.size > skin_bone_count)
        throw gpu::exception::base("Skin slice out of range for texture");

    if (u_skin_start != -1)
        gl_call(glUniform1i, u_skin_start, slice.begin);
    if (u_skin_count != -1)
        gl_call(glUniform1i, u_skin_count, slice.size);
}
void engine::gpu::shader::program::set_albedo_texture(
    const asset::texture &tex) const
{
    if (u_albedo_tex != -1)
    {
        tex.bind(COLOR_TEXTURE_UNIT);
        gl_call(glUniform1i, u_albedo_tex, (GLint)COLOR_TEXTURE_UNIT);
    }
}

void engine::gpu::shader::program::set_no_skin()
{
    if (u_skin_count != -1)
        gl_call(glUniform1i, u_skin_count, 0);
}

void engine::gpu::shader::program::set_model_transform(
    const vec::transform3 &transform)
{
    model = vec::fmat4_transform3(transform);
    if (u_model != -1)
    {
        gl_call(glUniformMatrix4fv,
                u_model,
                1,
                GL_FALSE,
                (const GLfloat *)&model);
    }

    if (u_mvp != -1)
    {
        vec::fmat4 mvp = view_projection * model;
        gl_call(glUniformMatrix4fv, u_mvp, 1, GL_FALSE, (const GLfloat *)&mvp);
    }

    if (u_normal != -1)
    {
        vec::fmat4_transform3_inverse inv(transform);
        vec::fmat3 normal_matrix = vec::transpose(vec::fmat3(inv));
        gl_call(glUniformMatrix3fv,
                u_normal,
                1,
                GL_FALSE,
                (const GLfloat *)&normal_matrix);
    }
}

void engine::gpu::shader::program::set_view(const vec::transform3 &transform)
{
    view = vec::fmat4_transform3_inverse(transform);
    if (u_view != -1)
    {
        gl_call(glUniformMatrix4fv,
                u_view,
                1,
                GL_FALSE,
                (const GLfloat *)&view);
    }
}

void engine::gpu::shader::program::set_perspective(const vec::perspective &p)
{
    projection = vec::fmat4_perspective(p.fovy, p.aspect);
    if (u_projection != -1)
    {
        gl_call(glUniformMatrix4fv,
                u_projection,
                1,
                GL_FALSE,
                (const GLfloat *)&projection);
    }
}

void engine::gpu::shader::program::set_view_perspective(
    const vec::transform3 &v,
    const vec::perspective &p)
{
    set_view(v);
    set_perspective(p);
    view_projection = projection * view;
}

void engine::gpu::skin::allocate_texture(uint32_t bone_count)
{
    gl_check_error();

    if (id)
    {
        free_texture();
    }

    gl_call(glGenTextures, 1, &id);
    gl_call(glBindTexture, GL_TEXTURE_2D, id);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);
    gl_call(glTexParameteri,
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);
    gl_call(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);
    gl_call(glTexStorage2D, GL_TEXTURE_2D, 1, GL_RGBA32F, bone_count * 4, 1);
    this->bone_count = bone_count;
}

void engine::gpu::skin::free_texture()
{
    if (id)
    {
        glDeleteTextures(1, &id);
        id = 0;
        bone_count = 0;
    }
}

engine::gpu::skin::skin() {}

engine::gpu::skin::~skin()
{
    free_texture();
}

void engine::gpu::skin::set_pose(const std::vector<vec::fmat4> &matrices)
{
    if (matrices.size() < bone_count)
        allocate_texture(matrices.size());

    gl_call(glBindTexture, GL_TEXTURE_2D, id);
    gl_call(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);
    gl_call(glTexSubImage2D,
            GL_TEXTURE_2D,
            0,
            0,
            0,
            matrices.size() * 4,
            1,
            GL_RGBA,
            GL_FLOAT,
            matrices.data());
}

void engine::gpu::skin::bind() const
{
    gl_call(glActiveTexture, GL_TEXTURE0 + (GLenum)POSE_TEXTURE_UNIT);
    gl_call(glBindTexture, GL_TEXTURE_2D, id);
}

engine::gpu::frame::light_point::light_point(const vec::fvec3 &position,
                                             const vec::fvec3 &color,
                                             float radius)
    : position(position), color(color), radius(radius)
{
#define threshold_intensity 1
    intensity = threshold_intensity * radius * radius;
}

std::filesystem::file_time_type
engine::gpu::cache::asset::get_mtime(const std::string &path)
{
    return std::filesystem::last_write_time(path);
}

// #define fovx_to_fovy(fovx, aspect) 2.0 * atan(tan(0.5 * fovx) / aspect)
// engine::gpu::frame::camera::camera(const vec::fvec3 &position,
//                                    const vec::fvec4 &rotation,
//                                    float _fovx,
//                                    float aspect_ratio)
//     : position(position), rotation(rotation), fovx(_fovx),
//       aspect_ratio(aspect_ratio)
// {
//     float fovy = fovx_to_fovy(fovx, aspect_ratio);
//     float half_fov_y = fovy / 2.0f;
//     float half_fov_x = fovx / 2.0f;

//     vec::fvec3 plane_normal_back = vec::fvec3(0, 0, 1);
//     vec::fvec3 plane_normal_right =
//         vec::fvec3(std::cos(half_fov_x), 0, std::sin(half_fov_x));
//     vec::fvec3 plane_normal_up =
//         vec::fvec3(0, std::cos(half_fov_y), std::sin(half_fov_y));

//     frustum_normal[0] = rotation * plane_normal_back;
//     frustum_normal[1] = rotation * plane_normal_right;
//     frustum_normal[2] =
//         rotation * vec::fvec3(-plane_normal_right.x, 0,
//         plane_normal_right.z);
//     frustum_normal[3] = rotation * plane_normal_up;
//     frustum_normal[4] =
//         rotation * vec::fvec3(0, -plane_normal_up.y, plane_normal_up.z);
// }

// bool engine::gpu::frame::camera::sphere_is_visible(const vec::fvec3
// &center,
//                                                    float radius) const
// {
//     vec::fvec3 to_sphere = center - position;

//     for (const vec::fvec3 &normal : frustum_normal)
//     {
//         if (vec::dot(to_sphere, normal) > radius)
//             return false;
//     }
//     return true;
// }

void engine::gpu::state::forward::start_depth_pass()
{
    gl_call(glEnable, GL_DEPTH_TEST);
    gl_call(glDepthFunc, GL_LESS);
    gl_call(glDepthMask, GL_TRUE);
    gl_call(glColorMask, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    gl_call(glEnable, GL_CULL_FACE);
    gl_call(glCullFace, GL_BACK);
    gl_call(glClear, GL_DEPTH_BUFFER_BIT);
}

void engine::gpu::state::forward::start_draw_pass()
{
    gl_call(glColorMask, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    gl_call(glEnable, GL_DEPTH_TEST);
    gl_call(glDepthFunc, GL_LEQUAL);
    gl_call(glDepthMask, GL_FALSE);
}