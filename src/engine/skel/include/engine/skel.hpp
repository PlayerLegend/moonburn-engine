#include <engine/gltf.hpp>
#include <engine/vec.hpp>
#include <variant>
#include <vector>

namespace skel
{

using bone_index = uint8_t;

using animation_sampler_output =
    std::variant<std::vector<vec::fvec3>,
                 std::vector<vec::fvec4>,
                 std::vector<vec::cubicspline<vec::fvec3>>,
                 std::vector<vec::cubicspline<vec::fvec4>>>;

class animation_sampler
{
  public:
    std::vector<float> input;
    animation_sampler_output output;
    gltf::animation_sampler_interpolation interpolation;
    animation_sampler(const gltf::animation_sampler &gltf_sampler);
};

class animation_bone
{
  public:
    const animation_sampler *translation;
    const animation_sampler *rotation;
    const animation_sampler *scale;
};

// using animation = std::unordered_map<std::string, animation_bone>;
class animation
{
  public:
    std::unordered_map<std::string, animation_bone> bones;
    std::vector<animation_sampler> samplers;
    animation(gltf::animation &gltf_animation, const gltf::gltf &gltf);
};

class armature_bone
{
  public:
    bone_index child;
    bone_index peer;
    bone_index parent;
    armature_bone() : child(-1), peer(-1), parent(-1) {}
    armature_bone(bone_index _child, bone_index _peer, bone_index _parent)
        : child(_child), peer(_peer), parent(_parent)
    {
    }
};

class armature
{
  public:
    std::unordered_map<std::string, bone_index> bones_names;
    std::string root_name;
    std::vector<vec::transform3> default_transforms;
    std::vector<vec::fmat4> inverse_bind_matrices;
    std::vector<armature_bone> bones;
    armature(const gltf::skin &gltf_skin, const gltf::gltf &gltf);
};

} // namespace skel