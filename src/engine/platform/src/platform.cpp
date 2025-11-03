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

namespace platform::button
{
std::string id_from_string(platform::button::id id)
{
    switch (id)
    {
    case platform::button::INVALID:
        return "INVALID";
    case platform::button::A:
        return "A";
    case platform::button::B:
        return "B";
    case platform::button::C:
        return "C";
    case platform::button::D:
        return "D";
    case platform::button::E:
        return "E";
    case platform::button::F:
        return "F";
    case platform::button::G:
        return "G";
    case platform::button::H:
        return "H";
    case platform::button::I:
        return "I";
    case platform::button::J:
        return "J";
    case platform::button::K:
        return "K";
    case platform::button::L:
        return "L";
    case platform::button::M:
        return "M";
    case platform::button::N:
        return "N";
    case platform::button::O:
        return "O";
    case platform::button::P:
        return "P";
    case platform::button::Q:
        return "Q";
    case platform::button::R:
        return "R";
    case platform::button::S:
        return "S";
    case platform::button::T:
        return "T";
    case platform::button::U:
        return "U";
    case platform::button::V:
        return "V";
    case platform::button::W:
        return "W";
    case platform::button::X:
        return "X";
    case platform::button::Y:
        return "Y";
    case platform::button::Z:
        return "Z";
    case platform::button::N0:
        return "0";
    case platform::button::N1:
        return "1";
    case platform::button::N2:
        return "2";
    case platform::button::N3:
        return "3";
    case platform::button::N4:
        return "4";
    case platform::button::N5:
        return "5";
    case platform::button::N6:
        return "6";
    case platform::button::N7:
        return "7";
    case platform::button::N8:
        return "8";
    case platform::button::N9:
        return "9";
    case platform::button::MINUS:
        return "MINUS";
    case platform::button::EQUAL:
        return "EQUAL";
    case platform::button::BACKSPACE:
        return "BACKSPACE";
    case platform::button::GRAVE:
        return "GRAVE";
    case platform::button::INSERT:
        return "INSERT";
    case platform::button::HOME:
        return "HOME";
    case platform::button::PGUP:
        return "PGUP";
    case platform::button::TAB:
        return "TAB";
    case platform::button::LEFT_BRACKET:
        return "LEFT_BRACKET";
    case platform::button::RIGHT_BRACKET:
        return "RIGHT_BRACKET";
    case platform::button::BACKSLASH:
        return "BACKSLASH";
    case platform::button::CAPSLOCK:
        return "CAPSLOCK";
    case platform::button::COMMA:
        return "COMMA";
    case platform::button::PERIOD:
        return "PERIOD";
    case platform::button::SEMICOLON:
        return "SEMICOLON";
    case platform::button::APOSTROPHE:
        return "APOSTROPHE";
    case platform::button::SLASH:
        return "SLASH";
    case platform::button::ENTER:
        return "ENTER";
    case platform::button::LEFT_SHIFT:
        return "LEFT_SHIFT";
    case platform::button::RIGHT_SHIFT:
        return "RIGHT_SHIFT";
    case platform::button::LEFT_CONTROL:
        return "LEFT_CONTROL";
    case platform::button::RIGHT_CONTROL:
        return "RIGHT_CONTROL";
    case platform::button::LEFT_ALT:
        return "LEFT_ALT";
    case platform::button::RIGHT_ALT:
        return "RIGHT_ALT";
    case platform::button::LEFT_SUPER:
        return "LEFT_SUPER";
    case platform::button::RIGHT_SUPER:
        return "RIGHT_SUPER";
    case platform::button::MOUSE1:
        return "MOUSE1";
    case platform::button::MOUSE2:
        return "MOUSE2";
    case platform::button::MOUSE3:
        return "MOUSE3";
    case platform::button::MOUSE4:
        return "MOUSE4";
    case platform::button::MOUSE5:
        return "MOUSE5";
    case platform::button::MOUSE6:
        return "MOUSE6";
    case platform::button::MOUSE7:
        return "MOUSE7";
    case platform::button::MOUSE8:
        return "MOUSE8";
    case platform::button::MOUSEWHEEL_UP:
        return "MOUSEWHEEL_UP";
    case platform::button::MOUSEWHEEL_DOWN:
        return "MOUSEWHEEL_DOWN";
    case platform::button::ESCAPE:
        return "ESCAPE";
    case platform::button::SPACE:
        return "SPACE";
    case platform::button::F1:
        return "F1";
    case platform::button::F2:
        return "F2";
    case platform::button::F3:
        return "F3";
    case platform::button::F4:
        return "F4";
    case platform::button::F5:
        return "F5";
    case platform::button::F6:
        return "F6";
    case platform::button::F7:
        return "F7";
    case platform::button::F8:
        return "F8";
    case platform::button::F9:
        return "F9";
    case platform::button::F10:
        return "F10";
    case platform::button::F11:
        return "F11";
    case platform::button::F12:
        return "F12";
    default:
        return "UNKNOWN";
    }
}

} // namespace platform::button