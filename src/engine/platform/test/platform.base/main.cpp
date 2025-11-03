#include <engine/platform.hpp>

int main()
{
    platform::window window("Platform Base Test");

    platform::frame::state frame = window.get_frame();

    return 0;
}