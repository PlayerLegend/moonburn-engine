#include "engine/gltf.hpp"
#include <engine/view3.hpp>

namespace engine::view3
{

asset::object::object(const engine::gpu::mesh &mesh,
                      const skel::armature *armature)
    : mesh(mesh), armature(armature)
{
}
asset::asset(const gltf::gltf &gltf)
{
    for (const gltf::mesh &mesh : gltf.meshes)
        meshes.emplace(mesh.name, engine::gpu::mesh(mesh));
    for (const gltf::skin &skin : gltf.skins)
        armatures.emplace(skin.name, skel::armature(skin, gltf));
    for (const gltf::animation &anim : gltf.animations)
        animations.emplace(anim.name, skel::animation(anim, gltf));
    for (const gltf::node &node : gltf.nodes)
    {
        const skel::armature *arm = NULL;
        if (!node.mesh)
            continue;
        auto mesh = meshes.find(node.mesh->name);
        if (mesh == meshes.end())
            continue;
        if (node.skin)
        {
            auto it = armatures.find(node.skin->name);
            if (it != armatures.end())
                arm = &it->second;
        }
        if (!node.mesh || !node.skin)
            continue;
        nodes.emplace(node.name, asset::object(&mesh->second, arm));
    }
}
} // namespace engine::view3