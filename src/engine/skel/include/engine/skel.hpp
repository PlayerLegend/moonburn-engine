#include <engine/gltf.hpp>
#include <engine/vec.hpp>
#include <variant>
#include <vector>

namespace skel
{

using bone_index = uint8_t;
inline constexpr bone_index max_bones = 255;

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
    armature_bone() : child(max_bones), peer(max_bones), parent(max_bones) {}
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

class result
{
    vec::fvec3 value_translation[max_bones];
    float weight_translation[max_bones];
    vec::fvec4 value_rotation[max_bones];
    float weight_rotation[max_bones];
    vec::fvec3 value_scale[max_bones];
    float weight_scale[max_bones];
    vec::fmat4 transforms[max_bones];

  public:
    result(const armature &armature);
    void accumulate_translation(bone_index bone,
                                const vec::fvec3 &translation,
                                float weight);
    void accumulate_rotation(bone_index bone,
                             const vec::fvec4 &rotation,
                             float weight);
    void
    accumulate_scale(bone_index bone, const vec::fvec3 &scale, float weight);
    vec::transform3 get_final_transform(bone_index bone) const;
};

} // namespace skel