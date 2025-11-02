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

// using animation_sampler_input = std::vector<float>;

class animation_sampler_input
{
  public:
    class frame_params
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
        frame_params(const std::vector<float> &times) : times(times) {}

        void update(float time);
    };

  private:
    std::vector<float> times;
    frame_params cache;

  public:
    animation_sampler_input(const gltf::accessor &accessor) : cache(times)
    {
        times = accessor;
    }
    float operator[](size_t index) const
    {
        return times[index];
    }
    size_t size() const
    {
        return times.size();
    }
    const float *data() const
    {
        return times.data();
    }
    const frame_params &get_frame_params(float time)
    {
        cache.update(time);
        return cache;
    }
};

class animation_sampler_input_hash
{
  public:
    size_t operator()(const animation_sampler_input &input) const
    {
        // sdbm
        std::size_t hash = 0;
        uint8_t *i = (uint8_t *)input.data();
        uint8_t *end = i + input.size() * sizeof(input[0]);

        while (i < end)
        {
            hash = (*i++) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }
};

class animation_sampler_input_equal
{
  public:
    bool operator()(const animation_sampler_input &a,
                    const animation_sampler_input &b) const
    {
        if (a.size() != b.size())
            return false;

        for (size_t i = 0; i < a.size(); i++)
        {
            if (std::abs(a[i] - b[i]) > vec::epsilon)
                return false;
        }

        return true;
    }
};

using animation_sampler_input_unordered_set =
    std::unordered_set<animation_sampler_input,
                       animation_sampler_input_hash,
                       animation_sampler_input_equal>;

template <typename T> class animation_sampler_step
{
    animation_sampler_input &input;
    const std::vector<T> &output;

  public:
    animation_sampler_step(animation_sampler_input_unordered_set &all_inputs,
                           const gltf::animation_sampler &gltf_sampler);
    T operator[](float);
};

template <typename T> class animation_sampler_linear
{
    animation_sampler_input &input;
    const std::vector<T> &output;

  public:
    animation_sampler_linear(animation_sampler_input_unordered_set &all_inputs,
                             const gltf::animation_sampler &gltf_sampler);
    T operator[](float);
};

template <typename T> class animation_sampler_cubicspline
{
    animation_sampler_input &input;
    const std::vector<vec::cubicspline<T>> &output;

  public:
    animation_sampler_cubicspline(
        animation_sampler_input_unordered_set &all_inputs,
        const gltf::animation_sampler &gltf_sampler);
    T operator[](float);
};

using animation_sampler =
    std::variant<animation_sampler_step<vec::fvec3>,
                 animation_sampler_step<vec::fvec4>,
                 animation_sampler_linear<vec::fvec3>,
                 animation_sampler_linear<vec::fvec4>,
                 animation_sampler_cubicspline<vec::fvec3>,
                 animation_sampler_cubicspline<vec::fvec4>>;

// class animation_sampler
// {
//   public:
//     const animation_sampler_input &input;
//     animation_sampler_output output;
//     gltf::animation_sampler_interpolation interpolation;
//     animation_sampler(animation_sampler_input_unordered_set &all_inputs,
//                       const gltf::animation_sampler &gltf_sampler);
// };

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

using animation_bone = std::vector<animation_channel>;

// using animation = std::unordered_map<std::string, animation_bone>;
class animation
{
  public:
    animation_sampler_input_unordered_set all_inputs;
    std::unordered_map<std::string, animation_bone> bones;
    std::vector<animation_sampler> samplers;
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

    result(const class armature &armature);
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
    operator std::vector<vec::fmat4> &();
    void accumulate(const std::string &root_name,
                    const skel::animation &animation,
                    float time,
                    float weight);
    void accumulate(const skel::animation &animation, float time, float weight)
    {
        accumulate(armature.root_name, animation, time, weight);
    }
};

} // namespace skel