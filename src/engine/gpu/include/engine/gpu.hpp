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

namespace skel
{
class pose;
} // namespace skel

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
    float radius = 0;
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

class target
{
    uint32_t fbo = 0;
    uint32_t color = 0;
    uint32_t depth_stencil = 0;

  public:
    target(uint32_t width, uint32_t height);
    target();
    ~target();

    void bind();
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

    void allocate_texture(uint32_t bone_count);
    void free_texture();

  public:
    uint32_t bone_count = 0;

    skin(uint32_t length);
    ~skin();

    void set_pose(const std::vector<vec::fmat4> &matrices);
    void operator=(const std::vector<vec::fmat4> &matrices)
    {
        set_pose(matrices);
    }

    void bind(uint32_t unit) const;
};

class mesh
{
    std::vector<primitive> primitives;

  public:
    float radius;
    mesh(const class gltf::mesh &mesh);
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
    int32_t u_model = -1;
    int32_t u_view = -1;
    int32_t u_projection = -1;
    int32_t u_normal = -1;
    int32_t u_mvp = -1;

    vec::fmat4 projection;
    vec::fmat4 view;
    vec::fmat4 model;

  public:
    program(const vertex *vertex_shader, const fragment *fragment_shader);
    ~program();

    void bind();
    void set_skin(const gpu::skin &skin);
    void set_model_transform(const vec::transform3 &);
    void set_view(const vec::transform3 &);
    void set_perspective(const vec::perspective &);
};

} // namespace engine::gpu::shader

namespace engine::gpu::frame
{
class actor
{
  public:
    engine::gpu::mesh &mesh;
    skel::pose &pose;
    vec::transform3 transform;

    actor(engine::gpu::mesh &mesh,
          skel::pose &pose,
          const vec::transform3 &transform)
        : mesh(mesh), pose(pose), transform(transform)
    {
    }
};

class light_point
{
  public:
    vec::fvec3 position;
    vec::fvec3 color;
    float intensity;
    float radius;
    light_point(const vec::fvec3 &position,
                const vec::fvec3 &color,
                float radius);
};

// class camera
// {
//     std::array<vec::fvec3, 5> frustum_normal;

//   public:
//     vec::fvec3 position;
//     vec::fvec4 rotation;
//     float fovx;
//     float aspect_ratio;
//     camera(const vec::fvec3 &position,
//            const vec::fvec4 &rotation,
//            float fovx,
//            float aspect_ratio);
//     bool sphere_is_visible(const vec::fvec3 &center, float radius) const;
// };

class queue
{
    std::vector<actor> actors;
    std::vector<light_point> lights;

  public:
    void operator+=(const actor &actor)
    {
        actors.push_back(actor);
    }
    void operator+=(const light_point &light)
    {
        lights.push_back(light);
    }
    // std::vector<actor> get_visible_actors(const camera &camera);
    // std::vector<light_point> get_visible_lights(const camera &camera);
};

} // namespace engine::gpu::frame