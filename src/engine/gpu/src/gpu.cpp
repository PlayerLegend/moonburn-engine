#include "engine/vec.hpp"
#include <array>
#include <cmath>
#include <engine/gltf.hpp>
#include <engine/gpu.hpp>
#include <glad/glad.h>

#define UNIFORM_NAME_SKIN "u_skin"
#define UNIFORM_NAME_SKIN_COUNT "u_skin_count"
#define UNIFORM_NAME_TRANSFORM "u_transform"
#define UNIFORM_NAME_NORMAL_MATRIX "u_normal_matrix"

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
            throw engine::gpu::exception::base(                                \
                "GL error " + std::to_string(error) +                          \
                " after calling function " #name);                             \
    }

engine::gpu::primitive::primitive(const class gltf::mesh_primitive &input)
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
          .component_type = gltf::component_type::USHORT,
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

        if (attribute.accessor->component_type == gltf::component_type::FLOAT)
        {
            gl_call(glVertexAttribPointer,
                    attribute.index,
                    (GLint)attribute.attribute_type,
                    (GLenum)attribute.component_type,
                    attribute.normalized,
                    0,
                    (void *)vertex_data.size());
        }
        else
        {
            gl_call(glVertexAttribIPointer,
                    attribute.index,
                    (GLint)attribute.attribute_type,
                    (GLenum)attribute.component_type,
                    0,
                    (void *)vertex_data.size());
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

engine::gpu::primitive::~primitive()
{
    if (ibo)
        glDeleteBuffers(1, &ibo);
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (vao)
        glDeleteVertexArrays(1, &vao);
}

void engine::gpu::primitive::bind()
{
    gl_call(glBindVertexArray, vao);
    gl_call(glBindBuffer, GL_ARRAY_BUFFER, vbo);
    if (ibo)
        gl_call(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void engine::gpu::primitive::draw()
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

engine::gpu::texture::texture(const gltf::texture &texture)
{
    gl_check_error();

    gl_call(glGenTextures, 1, &id);
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
}

engine::gpu::texture::~texture()
{
    if (id)
        glDeleteTextures(1, &id);
}

void engine::gpu::texture::bind(uint32_t unit)
{
    gl_call(glActiveTexture, GL_TEXTURE0 + (GLenum)unit);
    gl_call(glBindTexture, GL_TEXTURE_2D, id);
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
        throw engine::gpu::exception::base("Failed to compile shader: " + log);
    }
}

engine::gpu::shader::shader::~shader()
{
    if (id)
        glDeleteShader(id);
}

engine::gpu::shader::vertex::vertex::vertex(std::string source)
    : shader(GL_VERTEX_SHADER, source)
{
}

engine::gpu::shader::fragment::fragment::fragment(std::string source)
    : shader(GL_FRAGMENT_SHADER, source)
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

    u_skin = glGetUniformLocation(id, UNIFORM_NAME_SKIN);
    u_skin_count = glGetUniformLocation(id, UNIFORM_NAME_SKIN_COUNT);
    u_transform = glGetUniformLocation(id, UNIFORM_NAME_TRANSFORM);
    u_normal_matrix = glGetUniformLocation(id, UNIFORM_NAME_NORMAL_MATRIX);
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

void engine::gpu::shader::program::set_skin(
    const std::vector<vec::fmat4> &matrices)
{
    gl_call(glUniformMatrix4fv,
            u_skin,
            (GLsizei)matrices.size(),
            GL_FALSE,
            (const GLfloat *)matrices.data());
}

void engine::gpu::shader::program::set_transform(
    const vec::transform3 &transform)
{
    if (u_transform != -1)
    {
        vec::fmat4_transform3 model_matrix = transform;
        gl_call(glUniformMatrix4fv,
                u_transform,
                1,
                GL_FALSE,
                (const GLfloat *)&model_matrix);
    }

    if (u_normal_matrix != -1)
    {
        vec::fmat3 normal_matrix =
            transpose((vec::fmat3)vec::fmat4_transform3_inverse(transform));
        gl_call(glUniformMatrix3fv,
                u_normal_matrix,
                1,
                GL_FALSE,
                (const GLfloat *)&normal_matrix);
    }
}

void engine::gpu::skin::allocate_texture(uint32_t length)
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
    gl_call(glTexStorage2D, GL_TEXTURE_2D, 1, GL_RGBA32F, length * 4, 1);
    this->length = length;
}

void engine::gpu::skin::free_texture()
{
    if (id)
    {
        glDeleteTextures(1, &id);
        id = 0;
        length = 0;
    }
}

engine::gpu::skin::skin(uint32_t length)
{
    allocate_texture(length);
}

engine::gpu::skin::~skin()
{
    free_texture();
}

void engine::gpu::skin::set_pose(const std::vector<vec::fmat4> &matrices)
{
    if (matrices.size() < length)
        allocate_texture(matrices.size());

    gl_call(glBindTexture, GL_TEXTURE_2D, id);
    gl_call(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);
    gl_call(glTexSubImage2D,
            GL_TEXTURE_2D,
            0,
            0,
            0,
            length * 4,
            1,
            GL_RGBA,
            GL_FLOAT,
            matrices.data());
}

engine::gpu::frame::light_point::light_point(const vec::fvec3 &position,
                                             const vec::fvec3 &color,
                                             float radius)
    : position(position), color(color), radius(radius)
{
#define threshold_intensity 1
    intensity = threshold_intensity * radius * radius;
}

engine::gpu::frame::camera::camera(const vec::fvec3 &position,
                                   const vec::fvec4 &rotation,
                                   float fov_y,
                                   float aspect_ratio)
    : position(position), rotation(rotation), fov_y(fov_y),
      aspect_ratio(aspect_ratio)
{
    float fov_x = std::atan(std::tan(fov_y * 0.5f) * aspect_ratio) * 2.0f;
    float half_fov_y = fov_y / 2.0f;
    float half_fov_x = fov_x / 2.0f;

    vec::fvec3 plane_normal_back = vec::fvec3(0, 0, 1);
    vec::fvec3 plane_normal_right =
        vec::fvec3(std::cos(half_fov_x), 0, std::sin(half_fov_x));
    vec::fvec3 plane_normal_up =
        vec::fvec3(0, std::cos(half_fov_y), std::sin(half_fov_y));

    frustum_normal[0] = rotation * plane_normal_back;
    frustum_normal[1] = rotation * plane_normal_right;
    frustum_normal[2] =
        rotation * vec::fvec3(-plane_normal_right.x, 0, plane_normal_right.z);
    frustum_normal[3] = rotation * plane_normal_up;
    frustum_normal[4] =
        rotation * vec::fvec3(0, -plane_normal_up.y, plane_normal_up.z);
}

bool engine::gpu::frame::camera::sphere_is_visible(const vec::fvec3 &center,
                                                   float radius) const
{
    vec::fvec3 to_sphere = center - position;

    for (const vec::fvec3 &normal : frustum_normal)
    {
        if (vec::dot(to_sphere, normal) > radius)
            return false;
    }
    return true;
}