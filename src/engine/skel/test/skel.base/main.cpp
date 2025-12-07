#include <engine/gltf.hpp>
#include <engine/skel.hpp>
#include <filesystem>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "skel_test")
                  << " <path-to-glb>\n";
        return 1;
    }

    std::filesystem::path glb_path(argv[1]);

    engine::filesystem::whitelist wl(glb_path.parent_path().string());
    engine::filesystem::cache_binary fs_bin(wl);
    engine::image::cache::rgba32 fs_img(wl);

    gltf::gltf_cache cache(wl, fs_bin, fs_img);

    gltf::gltf_cache::reference ref = cache[glb_path];
    if (!ref)
    {
        std::cerr << "Failed to load glTF: " << glb_path << "\n";
        std::exit(1);
    }
    const gltf::gltf &doc = *ref;

    skel::armature armature(doc.get_skin(0), doc);

    skel::animation animation(doc.get_animation(0), doc);

    skel::pose pose;

    pose.start(armature);

    // pose += skel::frame(animation, 0.2f, 1.0f);
    pose.accumulate(animation, 0.2f, 0.2f);

    std::vector<vec::fmat4> matrices;
    skel::pose::slice set = pose.append_matrices(matrices);
    assert(set.begin == 0);
    assert(set.size == matrices.size());

    std::cout << "Got " << matrices.size() << " bone matrices\n";

    std::cout << "Skeleton root name: " << armature.root_name << "\n";

    std::cout << "skel test: OK\n";
    return 0;
}