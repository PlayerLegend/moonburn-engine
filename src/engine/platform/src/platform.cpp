#include <engine/platform.hpp>

namespace platform::frame
{
void state::clear()
{
    events.clear();
    clear_edge();
    mouse.delta = vec::fvec2{0.0f, 0.0f};
    mouse.begin = mouse.end;
}

void state::mouse_warp(const vec::fvec2 &position)
{
    vec::fvec2 begin = mouse.end;
    vec::fvec2 delta = position - begin;
    mouse.end = position;
    mouse.delta = delta;

    events.emplace_back(platform::event::pointer{begin, position, delta});
}

void state::mouse_delta(const vec::fvec2 &delta)
{
    vec::fvec2 begin = mouse.end;
    vec::fvec2 end = begin + delta;
    mouse.end = end;
    mouse.delta = delta;

    events.emplace_back(platform::event::pointer{begin, end, delta});
}

void state::button_press(enum platform::button::id id)
{
    if (id >= platform::button::id::MAX)
    {
        return;
    }

    buttons[id] |= (platform::button::PRESSED | platform::button::HELD);

    events.emplace_back(platform::event::button{id, platform::button::PRESSED});
    edge_list.push_back(id);
}

void state::button_release(enum platform::button::id id)
{
    if (id >= platform::button::id::MAX)
    {
        return;
    }

    buttons[id] |= platform::button::RELEASED;
    buttons[id] &= ~platform::button::HELD;

    events.emplace_back(
        platform::event::button{id, platform::button::RELEASED});
    edge_list.push_back(id);
}

void state::window_resize(const vec::vec2<unsigned int> &dimensions)
{
    window.dimensions = dimensions;
    window.aspect_ratio =
        static_cast<float>(dimensions.x) / static_cast<float>(dimensions.y);

    events.emplace_back(
        platform::event::window_resize{window.aspect_ratio, dimensions});
}

void state::clear_edge()
{
    for (enum platform::button::id id : edge_list)
    {
        if (id < platform::button::id::MAX)
        {
            buttons[id] &=
                ~(platform::button::PRESSED | platform::button::RELEASED);
        }
    }

    edge_list.clear();
}
} // namespace platform::frame
