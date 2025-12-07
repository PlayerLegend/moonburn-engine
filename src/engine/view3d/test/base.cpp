#include <assert.h>
#include <engine/platform.hpp>
#include <engine/view3.hpp>

#define DEG2RAD (3.14159 / 180.0)

int main(int argc, char *argv[])
{
    assert(argc == 4);
    std::string root = argv[1];
    std::string glb = argv[2];
    std::string shader = argv[3];

    platform::window window("view3 test");

    engine::view3::pipeline::forward pipeline(root);

    while (true)
    {
        platform::frame::state frame = window.get_frame();
        if (frame.should_close)
            break;
        vec::perspective perspective(DEG2RAD * 120, frame.window.aspect_ratio);
        vec::transform3 camera_transform(
            vec::fvec3(0, 0, 5),
            vec::fvec4(vec::fvec3(0, 0, 0), vec::up));

        vec::transform3 model_transform(
            vec::fvec3(0, 0, 0),
            vec::fvec4(vec::fvec3(0, 1, 0), frame.time.now));

        engine::view3::object obj = {
            model_transform,
            shader,
            glb,
        };

        pipeline += obj;

        pipeline.draw(camera_transform, perspective);
    }
}