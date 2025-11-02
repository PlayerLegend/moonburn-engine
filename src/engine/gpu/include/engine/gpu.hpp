#include <engine/exception.hpp>
#include <engine/vec.hpp>
#include <stdint.h>

namespace gltf
{
class mesh_primitive;
}

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
} // namespace engine::gpu

namespace engine::gpu::exception
{
class base : public engine::exception
{
  public:
    base(const std::string &message) : engine::exception(message) {}
};
} // namespace engine::gpu::exception