#include <engine/vec.hpp>
#include <string>

namespace engine::view3
{
class object
{
    vec::transform3 transform;
    vec::fmat4 transform_mat;
    bool transform_mat_stale = true;

  public:
    object() {};
    std::string mesh;
    std::string armature;
    void set_transform(vec::transform3);
};

void draw(const object &object, const vec::fmat4_view &view, const vec::fmat4_projection &proj);
}; // namespace engine::view3