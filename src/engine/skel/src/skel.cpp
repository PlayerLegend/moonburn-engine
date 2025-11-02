#include <cmath>
#include <engine/skel.hpp>
#include <stack>

static skel::bone_index find_index_of_node(const gltf::node *node,
                                           const gltf::skin &skin)
{
    if (!node)
        return skel::max_bones;
    for (size_t joint_index = 0; joint_index < skin.joints.size();
         joint_index++)
    {
        if (node == skin.joints[joint_index])
            return joint_index;
    }
    throw skel::exception("Node " + node->name + " is not a joint in the skin");
}

static void armature_bone_add_child(std::vector<skel::armature_bone> &bones,
                                    skel::bone_index parent_index,
                                    skel::bone_index child_index)
{
    skel::armature_bone &parent_bone = bones[parent_index];
    skel::armature_bone &child_bone = bones[child_index];
    assert(child_bone.parent == skel::max_bones);
    assert(child_bone.peer == skel::max_bones);
    child_bone.peer = parent_bone.child;
    child_bone.parent = parent_index;
    parent_bone.child = child_index;
}

std::string find_skin_root_name(const gltf::skin &gltf_skin)
{
    const class gltf::node *root = gltf_skin.joints[0];
    while (root->parent)
        root = root->parent;
    return root->name;
}

skel::armature::armature(const gltf::skin &gltf_skin, const gltf::gltf &gltf)
{
    if (gltf_skin.inverse_bind_matrices)
    {
        inverse_bind_matrices =
            gltf_skin.inverse_bind_matrices->operator std::vector<vec::fmat4>();
    }
    else
    {
        inverse_bind_matrices.resize(gltf_skin.joints.size());
    }

    default_transforms.reserve(gltf_skin.joints.size());
    bones.resize(gltf_skin.joints.size());

    for (size_t joint_index = 0; joint_index < gltf_skin.joints.size();
         joint_index++)
    {
        const gltf::node &joint_node = *gltf_skin.joints[joint_index];
        bones_names[joint_node.name] = joint_index;
        default_transforms.push_back(joint_node.transform);
        for (const gltf::node *child_node : joint_node.children)
        {
            skel::bone_index child_index =
                find_index_of_node(child_node, gltf_skin);
            armature_bone_add_child(bones, joint_index, child_index);
        }
    }

    if (default_transforms.size() != inverse_bind_matrices.size())
        throw skel::exception(
            "Skin joint count does not match inverse bind matrix count");

    root_name = find_skin_root_name(gltf_skin);
}

bool input_times_compar(std::vector<float> &a, std::vector<float> &b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0, size = a.size(); i < size; i++)
    {
        if (std::abs(a[i] - b[i]) > vec::epsilon)
            return false;
    }
    return true;
}

static size_t frame_find(const std::vector<float> &times, float time)
{
    if (times.empty())
        throw skel::exception("No keyframes in animation sampler");

    int low = 0;
    int high = times.size() - 1;
    int mid;

    std::vector<float>::const_iterator frame;

    while (low < high)
    {
        mid = (low + high) / 2;

        frame = times.begin() + mid;

        if (frame[0] > time)
        {
            high = mid - 1;
        }
        else if (frame[1] < time)
        {
            low = mid + 1;
        }
        else
        {
            low = mid;
            break;
        }
    }

    return low;
}

template <typename T>
skel::animation_sampler_step<T>::animation_sampler_step(
    animation_sampler_input_unordered_set &all_inputs,
    const gltf::animation_sampler &gltf_sampler)
    : input(*all_inputs.emplace(gltf_sampler.input).first),
      output(gltf_sampler.output)
{
}

// template <typename T>
// T skel::animation_sampler_step<T>::operator[](float time) const
// {
//     size_t frame_index = frame_find(input, time);
//     return output[frame_index];
// }

template <typename T>
skel::animation_sampler_linear<T>::animation_sampler_linear(
    animation_sampler_input_unordered_set &all_inputs,
    const gltf::animation_sampler &gltf_sampler)
    : input(*all_inputs.emplace(gltf_sampler.input).first),
      output(gltf_sampler.output)
{
}

template <>
vec::fvec3 skel::animation_sampler_linear<vec::fvec3>::operator[](float time)
{
    const animation_sampler_input::frame_params &params =
        input.get_frame_params(time);

    if (params.clamp)
    {
        return output.back();
    }

    const vec::fvec3 &v0 = output[params.i_before];
    const vec::fvec3 &v1 = output[params.i_before + 1];
    return v0 * params.t_inv + v1 * params.t;
}

template <>
vec::fvec4 skel::animation_sampler_linear<vec::fvec4>::operator[](float time)
{
    const animation_sampler_input::frame_params &params =
        input.get_frame_params(time);

    if (params.clamp)
    {
        return output.back();
    }

    const vec::fvec4 &v0 = output[params.i_before];
    const vec::fvec4 &v1 = output[params.i_before + 1];
    return vec::slerp(v0, v1, params.t);
}

template <typename T>
skel::animation_sampler_cubicspline<T>::animation_sampler_cubicspline(
    animation_sampler_input_unordered_set &all_inputs,
    const gltf::animation_sampler &gltf_sampler)
    : input(*all_inputs.emplace(gltf_sampler.input).first),
      output(gltf_sampler.output)
{
}

template <>
vec::fvec3
skel::animation_sampler_cubicspline<vec::fvec3>::operator[](float time)
{
    const animation_sampler_input::frame_params &params =
        input.get_frame_params(time);

    if (params.clamp)
    {
        return output.back().value;
    }

    const vec::cubicspline<vec::fvec3> &s0 = output[params.i_before];
    const vec::cubicspline<vec::fvec3> &s1 = output[params.i_before + 1];

    return s0.value * params.h00 + s0.out_tangent * params.h10 +
           s1.value * params.h01 + s1.in_tangent * params.h11;
}

template <>
vec::fvec4
skel::animation_sampler_cubicspline<vec::fvec4>::operator[](float time)
{
    const animation_sampler_input::frame_params &params =
        input.get_frame_params(time);

    if (params.clamp)
    {
        return output.back().value;
    }

    const vec::cubicspline<vec::fvec4> &s0 = output[params.i_before];
    const vec::cubicspline<vec::fvec4> &s1 = output[params.i_before + 1];

    return s0.value * params.h00 + s0.out_tangent * params.h10 +
           s1.value * params.h01 + s1.in_tangent * params.h11;
}

void skel::animation_sampler_input::frame_params::update(float time)
{
    if (std::fabs(tc - time) < vec::epsilon)
        return;

    tc = time;

    i_before = frame_find(times, time);

    if (i_before == times.size() - 1)
    {
        clamp = true;
        return;
    }

    clamp = false;

    t_before = times[i_before];
    t_after = times[i_before + 1];
    td = t_after - t_before;
    t = (time - t_before) / td;
    t_inv = 1.0f - t;

    h00 = 2 * t * t * t - 3 * t * t + 1;
    h10 = t * t * t - 2 * t * t + t;
    h01 = -2 * t * t * t + 3 * t * t;
    h11 = t * t * t - t * t;
}

static void animation_add_channel(skel::animation &animation,
                                  const gltf::animation &input_animation,
                                  const gltf::animation_channel &input_channel)
{
    if (!input_channel.target.node)
        return;

    skel::animation_bone &bone =
        animation.bones[input_channel.target.node->name];
    skel::animation_sampler &sampler = animation.samplers.at(
        input_animation.get_sampler_index(input_channel.sampler));

    bone.push_back(skel::animation_channel(input_channel.target.path, sampler));
}

skel::animation::animation(const gltf::animation &gltf_animation,
                           const gltf::gltf &gltf)
{
    samplers.reserve(gltf_animation.samplers.size());

    for (const gltf::animation_sampler &gltf_sampler : gltf_animation.samplers)
    {
        if (gltf_sampler.interpolation ==
            gltf::animation_sampler_interpolation::STEP)
        {
            if (gltf_sampler.output.type == gltf::attribute_type::VEC3)
                samplers.push_back(
                    skel::animation_sampler_step<vec::fvec3>(all_inputs,
                                                             gltf_sampler));
            else if (gltf_sampler.output.type == gltf::attribute_type::VEC4)
                samplers.push_back(
                    skel::animation_sampler_step<vec::fvec4>(all_inputs,
                                                             gltf_sampler));
            else
                throw skel::exception(
                    "Unsupported accessor type for animation sampler output");
        }
        else if (gltf_sampler.interpolation ==
                 gltf::animation_sampler_interpolation::LINEAR)
        {
            if (gltf_sampler.output.type == gltf::attribute_type::VEC3)
                samplers.push_back(
                    skel::animation_sampler_linear<vec::fvec3>(all_inputs,
                                                               gltf_sampler));
            else if (gltf_sampler.output.type == gltf::attribute_type::VEC4)
                samplers.push_back(
                    skel::animation_sampler_linear<vec::fvec4>(all_inputs,
                                                               gltf_sampler));
            else
                throw skel::exception(
                    "Unsupported accessor type for animation sampler output");
        }
        else if (gltf_sampler.interpolation ==
                 gltf::animation_sampler_interpolation::CUBICSPLINE)
        {
            if (gltf_sampler.output.type == gltf::attribute_type::VEC3)
                samplers.push_back(
                    skel::animation_sampler_cubicspline<vec::fvec3>(
                        all_inputs,
                        gltf_sampler));
            else if (gltf_sampler.output.type == gltf::attribute_type::VEC4)
                samplers.push_back(
                    skel::animation_sampler_cubicspline<vec::fvec4>(
                        all_inputs,
                        gltf_sampler));
            else
                throw skel::exception(
                    "Unsupported accessor type for animation sampler output");
        }
        else
        {
            throw skel::exception("Unsupported interpolation type");
        }
    }

    for (const gltf::animation_channel &gltf_channel : gltf_animation.channels)
    {
        animation_add_channel(*this, gltf_animation, gltf_channel);
    }
}

skel::result::result(const skel::armature &_armature) : armature(_armature)
{
    clear();
}

void skel::result::clear()
{
    transforms = armature.default_transforms;
    weights.resize(0);
    weights.resize(transforms.size());
}

void skel::result::accumulate_translation(bone_index bone,
                                          const vec::fvec3 &translation,
                                          float weight)
{
    vec::fvec3 &current_value = transforms[bone].translation;
    float &current_weight = weights[bone].translation;

    if (current_weight < vec::epsilon)
    {
        current_value = translation;
        current_weight = weight;
    }
    else
    {
        current_value = vec::lerp(current_value,
                                  translation,
                                  weight / (weight + current_weight));
        current_weight += weight;
    }
}

void skel::result::accumulate_rotation(bone_index bone,
                                       const vec::fvec4 &rotation,
                                       float weight)
{
    vec::fvec4 &current_value = transforms[bone].rotation;
    float &current_weight = weights[bone].rotation;

    if (current_weight < vec::epsilon)
    {
        current_value = rotation;
        current_weight = weight;
    }
    else
    {
        current_value = vec::slerp(current_value,
                                   rotation,
                                   weight / (weight + current_weight));
        current_weight += weight;
    }
}

void skel::result::accumulate_scale(bone_index bone,
                                    const vec::fvec3 &scale,
                                    float weight)
{
    vec::fvec3 &current_value = transforms[bone].scale;
    float &current_weight = weights[bone].scale;

    if (current_weight < vec::epsilon)
    {
        current_value = scale;
        current_weight = weight;
    }
    else
    {
        current_value =
            vec::lerp(current_value, scale, weight / (weight + current_weight));
        current_weight += weight;
    }
}

skel::result::operator std::vector<vec::fmat4> &()
{
    output.resize(0);
    output.reserve(transforms.size());

    for (const vec::transform3 &transform : transforms)
    {
        output.push_back(vec::fmat4_transform3(transform));
    }

    for (size_t i = 0, size = output.size(); i < size; i++)
    {
        const skel::armature_bone &bone = armature.bones[i];
        if (bone.parent != skel::max_bones)
        {
            output[i] = output[bone.parent] * output[i];
        }
    }

    for (size_t i = 0, size = output.size(); i < size; i++)
    {
        output[i] = output[i] * armature.inverse_bind_matrices[i];
    }

    return output;
}

void skel::result::accumulate(const std::string &root_name,
                              const skel::animation &animation,
                              float time,
                              float weight)
{
    std::stack<skel::bone_index> bone_stack;

    bone_stack.push(armature.bones_names.at(root_name));

    while (!bone_stack.empty())
    {
        skel::bone_index bone_index = bone_stack.top();
        bone_stack.pop();

        const skel::armature_bone &bone = armature.bones.at(bone_index);

        auto bone_it =
            animation.bones.find(armature.bones_names.begin()->first);
        if (bone_it == animation.bones.end())
            continue;
        const skel::animation_bone &animation_bone = bone_it->second;
        for (const skel::animation_channel &channel : animation_bone)
        {
            if (channel.path == gltf::animation_channel_path::TRANSLATION)
            {
                accumulate_translation(
                    bone_index,
                    std::get<skel::animation_sampler_step<vec::fvec3>>(
                        channel.sampler)[time],
                    weight);
            }
            else if (channel.path == gltf::animation_channel_path::ROTATION)
            {
                accumulate_rotation(
                    bone_index,
                    std::get<skel::animation_sampler_step<vec::fvec4>>(
                        channel.sampler)[time],
                    weight);
            }
            else if (channel.path == gltf::animation_channel_path::SCALE)
            {
                accumulate_scale(
                    bone_index,
                    std::get<skel::animation_sampler_step<vec::fvec3>>(
                        channel.sampler)[time],
                    weight);
            }
        }

        if (bone.child != skel::max_bones)

            bone_stack.push(bone.child);
        if (bone.peer != skel::max_bones)
            bone_stack.push(bone.peer);
    }
}