#include <engine/platform.hpp>
#include <iostream>

int main()
{
    platform::window window("Platform Base Test");

    while (true)
    {
        platform::frame::state frame = window.get_frame();
        if (frame.should_close)
            break;

        for (auto event : frame.events)
        {
            if (!std::holds_alternative<platform::event::button>(event))
                continue;

            platform::event::button &button_event =
                std::get<platform::event::button>(event);

            std::cout << "Button "
                      << platform::button::id_from_string(button_event.id)
                      << " is "
                      << (button_event.state & platform::button::PRESSED
                              ? "PRESSED"
                              : "RELEASED")
                      << std::endl;
        }
    }

    return 0;
}