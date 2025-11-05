#include <array>
#include <engine/gltf.hpp>
#include <engine/gpu.hpp>
#include <glad/glad.h>

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