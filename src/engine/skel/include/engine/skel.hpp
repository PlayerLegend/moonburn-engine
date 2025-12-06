#pragma once

#include <engine/gltf.hpp>
#include <engine/vec.hpp>
#include <variant>
#include <vector>

namespace skel
{
class exception : public engine::exception
{
  public:
    exception(const std::string &message) : engine::exception(message) {}
};

using bone_index = uint8_t;
inline constexpr bone_index max_bones = 255;

using animation_sampler_output =
    std::variant<std::vector<vec::fvec3>,
                 std::vector<vec::fvec4>,
                 std::vector<vec::cubicspline<vec::fvec3>>,
                 std::vector<vec::cubicspline<vec::fvec4>>>;

class interpolation_params
{
  public:
    float tc;
    float td;
    float t;
    float t_inv;

    float t_before;
    float t_after;

    size_t i_before;

    // Hermite basis (blending) functions
    float h00;
    float h10;
    float h01;
    float h11;

    bool clamp;

    const std::vector<float> &times;

    interpolation_params(const std::vector<float> &_times, float time);
};

template <typename T> class animation_sampler_step
{
    const std::vector<T> output;

  public:
    animation_sampler_step(const gltf::animation_sampler &gltf_sampler);
    T operator[](const interpolation_params &) const;
};

template <typename T> class animation_sampler_linear
{
    const std::vector<T> output;

  public:
    animation_sampler_linear(const gltf::animation_sampler &gltf_sampler);
    T operator[](const interpolation_params &) const;
};

template <typename T> class animation_sampler_cubicspline
{
    const std::vector<vec::cubicspline<T>> output;

  public:
    animation_sampler_cubicspline(const gltf::animation_sampler &gltf_sampler);
    T operator[](const interpolation_params &) const;
};

using animation_sampler =
    std::variant<animation_sampler_step<vec::fvec3>,
                 animation_sampler_step<vec::fvec4>,
                 animation_sampler_linear<vec::fvec3>,
                 animation_sampler_linear<vec::fvec4>,
                 animation_sampler_cubicspline<vec::fvec3>,
                 animation_sampler_cubicspline<vec::fvec4>>;

class animation_channel
{
  public:
    enum gltf::animation_channel_path path;
    const animation_sampler &sampler;
    animation_channel(enum gltf::animation_channel_path path,
                      const animation_sampler &sampler)
        : path(path), sampler(sampler)
    {
    }
};

class animation_times
{
  public:
    std::vector<float> input;
    std::unordered_map<std::string, std::vector<animation_channel>> bones;
    animation_times(const gltf::accessor &input_accessor);
};

class animation
{
    std::vector<animation_sampler> samplers;

    void add_channel(size_t input_index,
                     const gltf::animation &input_animation,
                     const gltf::animation_channel &input_channel);

  public:
    std::vector<animation_times> times;
    animation(const gltf::animation &gltf_animation, const gltf::gltf &gltf);
};

class armature_bone
{
  public:
    std::string name;
    bone_index child;
    bone_index peer;
    bone_index parent;
    armature_bone() : child(max_bones), peer(max_bones), parent(max_bones) {}
    armature_bone(const std::string &_name,
                  bone_index _child,
                  bone_index _peer,
                  bone_index _parent)
        : name(_name), child(_child), peer(_peer), parent(_parent)
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

class frame
{
  public:
    std::string root;
    const skel::animation &animation;
    float time;
    float weight;
    frame(const std::string &root,
          const skel::animation &animation,
          float time,
          float weight)
        : root(root), animation(animation), time(time), weight(weight)
    {
    }
    frame(const skel::animation &animation, float time, float weight)
        : animation(animation), time(time), weight(weight)
    {
    }
};

class pose
{
    class transform_weight
    {
      public:
        float translation;
        float rotation;
        float scale;
        transform_weight() : translation(0), rotation(0), scale(0) {}
    };

    std::vector<transform_weight> weights;
    std::vector<vec::transform3> transforms;
    std::vector<vec::fmat4> output;
    const skel::armature &armature;

    void accumulate_translation(bone_index bone,
                                const vec::fvec3 &translation,
                                float weight);
    void accumulate_rotation(bone_index bone,
                             const vec::fvec4 &rotation,
                             float weight);
    void
    accumulate_scale(bone_index bone, const vec::fvec3 &scale, float weight);

  public:
    void clear();
    operator const std::vector<vec::fmat4> &();
    void accumulate(const std::string &root_name,
                    const skel::animation &animation,
                    float time,
                    float weight);
    void accumulate(const skel::animation &animation, float time, float weight)
    {
        accumulate(armature.root_name, animation, time, weight);
    }
    void accumulate(const frame &_frame)
    {
        accumulate(_frame.root.empty() ? armature.root_name : _frame.root,
                   _frame.animation,
                   _frame.time,
                   _frame.weight);
    }
    void operator+=(const frame &_frame)
    {
        accumulate(_frame);
    }
    pose(const class armature &armature);
};

} // namespace skel