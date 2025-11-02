#include <engine/skel.hpp>

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

skel::animation_sampler::animation_sampler(
    animation_sampler_input_unordered_set &all_inputs,
    const gltf::animation_sampler &gltf_sampler)
    : input(*all_inputs.emplace(gltf_sampler.input).first),
      interpolation(gltf_sampler.interpolation)
{
    if (gltf_sampler.output.type == gltf::attribute_type::VEC3)
    {
        if (interpolation == gltf::animation_sampler_interpolation::CUBICSPLINE)
            output =
                (std::vector<vec::cubicspline<vec::fvec3>>)gltf_sampler.output;
        else if (interpolation ==
                     gltf::animation_sampler_interpolation::LINEAR ||
                 interpolation == gltf::animation_sampler_interpolation::STEP)
            output = (std::vector<vec::fvec3>)gltf_sampler.output;
        else
            throw skel::exception(
                "Unsupported interpolation type for VEC3 output");
    }

    if (gltf_sampler.output.type == gltf::attribute_type::VEC4)
    {
        if (interpolation == gltf::animation_sampler_interpolation::CUBICSPLINE)
            output =
                (std::vector<vec::cubicspline<vec::fvec4>>)gltf_sampler.output;
        else if (interpolation ==
                     gltf::animation_sampler_interpolation::LINEAR ||
                 interpolation == gltf::animation_sampler_interpolation::STEP)
            output = (std::vector<vec::fvec4>)gltf_sampler.output;
        else
            throw skel::exception(
                "Unsupported interpolation type for VEC4 output");
    }
    else
    {
        throw skel::exception(
            "Unsupported accessor type for animation sampler output");
    }
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

skel::animation::animation(gltf::animation &gltf_animation,
                           const gltf::gltf &gltf)
{
    samplers.reserve(gltf_animation.samplers.size());

    for (const gltf::animation_sampler &gltf_sampler : gltf_animation.samplers)
    {
        samplers.push_back(skel::animation_sampler(all_inputs, gltf_sampler));
    }

    for (const gltf::animation_channel &gltf_channel : gltf_animation.channels)
    {
        animation_add_channel(*this, gltf_animation, gltf_channel);
    }
}

// static 
size_t frame_find(const std::vector<float> &times, float time)
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

skel::result::result(const skel::armature &armature)
    : weight_translation(), weight_rotation(), weight_scale()
{
    for (size_t i = 0; i < armature.bones.size(); i++)
    {
        value_translation[i] = armature.default_transforms[i].translation;
        value_rotation[i] = armature.default_transforms[i].rotation;
        value_scale[i] = armature.default_transforms[i].scale;
    }
}

void skel::result::accumulate_translation(bone_index bone,
                                          const vec::fvec3 &translation,
                                          float weight)
{
    if (weight_translation[bone] < vec::epsilon)
    {
        value_translation[bone] = translation;
        weight_translation[bone] = weight;
    }
    else
    {
        value_translation[bone] =
            vec::lerp(value_translation[bone],
                      translation,
                      weight / (weight + weight_translation[bone]));
        weight_translation[bone] += weight;
    }
}

void skel::result::accumulate_rotation(bone_index bone,
                                       const vec::fvec4 &rotation,
                                       float weight)
{
    if (weight_rotation[bone] < vec::epsilon)
    {
        value_rotation[bone] = rotation;
        weight_rotation[bone] = weight;
    }
    else
    {
        value_rotation[bone] =
            vec::slerp(value_rotation[bone],
                       rotation,
                       weight / (weight + weight_rotation[bone]));
        weight_rotation[bone] += weight;
    }
}

void skel::result::accumulate_scale(bone_index bone,
                                    const vec::fvec3 &scale,
                                    float weight)
{
    if (weight_scale[bone] < vec::epsilon)
    {
        value_scale[bone] = scale;
        weight_scale[bone] = weight;
    }
    else
    {
        value_scale[bone] = vec::lerp(value_scale[bone],
                                      scale,
                                      weight / (weight + weight_scale[bone]));
        weight_scale[bone] += weight;
    }
}

vec::transform3 skel::result::get_final_transform(bone_index bone) const
{
    return vec::transform3(value_translation[bone],
                           value_rotation[bone],
                           value_scale[bone]);
}

// const vec::fmat4 *skel::result::get_transforms() const {}
