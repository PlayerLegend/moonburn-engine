#include "engine/vec.hpp"
#include <assert.h>
#include <engine/gltf.hpp>
#include <engine/gpu.hpp>
#include <engine/platform.hpp>

#define DEG2RAD (3.14159 / 180.0)

int main(int argc, char *argv[])
{
    assert(argc == 4);
    const std::string glb_path = argv[1];
    const std::string vs_path = argv[2];
    const std::string fs_path = argv[3];

    engine::filesystem::allocation vs_alloc(vs_path);
    engine::filesystem::allocation fs_alloc(fs_path);

    const std::string root =
        std::filesystem::path(glb_path).parent_path().string();

    engine::filesystem::whitelist wl(root);
    engine::filesystem::cache_binary fs_bin(wl);

    engine::image::cache::rgba32 fs_img(wl);
    gltf::gltf_cache cache(wl, fs_bin, fs_img);
    gltf::gltf_cache::reference ref = cache[glb_path];
    assert(ref);
    const gltf::gltf &doc = *ref;

    platform::window window("gpu test");

    engine::gpu::shader::vertex vertex_shader(
        std::string(vs_alloc.begin(), vs_alloc.end()));
    engine::gpu::shader::fragment fragment_shader(
        std::string(fs_alloc.begin(), fs_alloc.end()));
    engine::gpu::mesh mesh(doc.get_mesh(0));
    engine::gpu::target screen;

    while (true)
    {
        platform::frame::state frame = window.get_frame();
        if (frame.should_close)
            break;
        vec::perspective perspective(DEG2RAD * 120, frame.window.aspect_ratio);
        vec::transform3 camera_transform(
            vec::fvec3(0, 0, 5),
            vec::fvec4(vec::fvec3(0, 0, 0), vec::up));
    }
}
