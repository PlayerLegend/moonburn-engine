#include <engine/gltf.hpp>
#include <engine/gpu.hpp>
#include <engine/skel.hpp>
#include <engine/vec.hpp>

namespace engine::view3
{
class object
{
    vec::transform3 transform;
    vec::fmat4 transform_mat;
    bool transform_mat_stale = true;

  public:
    const engine::gpu::mesh &mesh;
    const skel::armature *armature;

    object(const engine::gpu::mesh &mesh, const skel::armature *armature)
        : mesh(mesh), armature(armature)
    {
    }
    void set_transform(vec::transform3);
};
class light_point;
class scene;
struct asset
{
    struct object
    {
        vec::transform3 transform;
        const engine::gpu::mesh &mesh;
        const skel::armature *armature;
        object(const engine::gpu::mesh &mesh, const skel::armature *armature);
    };

    std::unordered_map<std::string, engine::gpu::mesh> meshes;
    std::unordered_map<std::string, skel::armature> armatures;
    std::unordered_map<std::string, skel::animation> animations;
    std::unordered_map<std::string, asset::object> nodes;

    asset(const gltf::gltf &gltf);
};
}; // namespace engine::view3