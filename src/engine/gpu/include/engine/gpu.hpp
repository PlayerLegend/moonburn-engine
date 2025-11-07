#pragma once

#include <engine/exception.hpp>
#include <engine/image.hpp>
#include <engine/vec.hpp>
#include <stdint.h>
#include <vector>

namespace gltf
{
class mesh_primitive;
class texture;
class mesh;
} // namespace gltf

namespace engine::gpu::attributes
{
using position = vec::fvec3;
using normal = vec::vec3<int16_t>;
using tangent = vec::vec4<int16_t>;
using texcoord = vec::vec2<uint16_t>;
using color = vec::vec4<uint8_t>;
using joints = vec::vec4<uint8_t>;
using weights = vec::vec4<uint8_t>;
using index = uint32_t;
} // namespace engine::gpu::attributes

namespace engine::gpu
{
class primitive
{
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ibo = 0;
    uint32_t count = 0;
    bool short_indices = false;

  public:
    primitive(const class gltf::mesh_primitive &primitive);
    ~primitive();

    void draw();
    void bind();
};

class texture
{
    uint32_t id = 0;

  public:
    texture(const gltf::texture &texture);

    ~texture();
    void bind(uint32_t unit);
};

class gbuffer
{
    uint32_t fbo = 0;
    uint32_t position = 0;
    uint32_t normal = 0;
    uint32_t albedo_specular = 0;
    uint32_t depth_stencil = 0;

  public:
    gbuffer(uint32_t width, uint32_t height);
    ~gbuffer();

    void bind();
};

class skin
{
    uint32_t id = 0;
    uint32_t length = 0;

    void allocate_texture(uint32_t length);
    void free_texture();

  public:
    skin(uint32_t length);
    ~skin();

    void set_pose(const std::vector<vec::fmat4> &matrices);
    void operator=(const std::vector<vec::fmat4> &matrices)
    {
        set_pose(matrices);
    }

    void bind(uint32_t unit);
};

class mesh
{
    std::vector<primitive> primitives;
    class skin skin;

  public:
    mesh(const class gltf::mesh &mesh);

    void operator=(const std::vector<vec::fmat4> &matrices)
    {
        skin.set_pose(matrices);
    }
};

} // namespace engine::gpu

namespace engine::gpu::exception
{
class base : public engine::exception
{
  public:
    base(const std::string &message) : engine::exception(message) {}
};
} // namespace engine::gpu::exception

namespace engine::gpu::shader
{
class shader
{
    uint32_t id = 0;

  public:
    shader(uint32_t type, std::string source);
    ~shader();
    operator uint32_t() const
    {
        return id;
    }
};

class vertex : public shader
{
  public:
    vertex(std::string source);
};

class fragment : public shader
{
  public:
    fragment(std::string source);
};

class program
{
    uint32_t id = 0;
    int32_t u_skin = -1;
    int32_t u_skin_count = -1;
    int32_t u_transform = -1;
    int32_t u_normal_matrix = -1;

  public:
    program(const vertex *vertex_shader, const fragment *fragment_shader);
    ~program();

    void bind();
    void set_skin(const std::vector<vec::fmat4> &matrices);
    void set_transform(const vec::transform3 &transform);
};

} // namespace engine::gpu::shader