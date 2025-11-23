#include "engine/gltf.hpp"
#include <engine/view3.hpp>

namespace engine::view3
{
asset::asset(const gltf::gltf &gltf)
{
    for (const gltf::mesh &mesh : gltf.meshes)
        meshes.emplace(mesh.name, engine::gpu::mesh(mesh));
    for (const gltf::skin &skin : gltf.skins)
        armatures.emplace(skin.name, skel::armature(skin, gltf));
    for (const gltf::animation & anim : gltf.animations)
        animations.emplace(anim.name, skel::animation(anim, gltf));
    

}
} // namespace engine::view3