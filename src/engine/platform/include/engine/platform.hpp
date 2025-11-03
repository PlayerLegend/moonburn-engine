#include <engine/vec.hpp>
#include <memory>
#include <variant>
#include <vector>
#include <string>

namespace platform::button
{
enum id : uint8_t
{
    INVALID,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    N0,
    N1,
    N2,
    N3,
    N4,
    N5,
    N6,
    N7,
    N8,
    N9,
    MINUS,
    EQUAL,
    BACKSPACE,
    GRAVE,
    INSERT,
    HOME,
    PGUP,
    TAB,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    BACKSLASH,
    CAPSLOCK,
    COMMA,
    PERIOD,
    SEMICOLON,
    APOSTROPHE,
    SLASH,
    ENTER,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LEFT_CONTROL,
    RIGHT_CONTROL,
    LEFT_ALT,
    RIGHT_ALT,
    LEFT_SUPER,
    RIGHT_SUPER,
    MOUSE1,
    MOUSE2,
    MOUSE3,
    MOUSE4,
    MOUSE5,
    MOUSE6,
    MOUSE7,
    MOUSE8,
    ESCAPE,
    SPACE,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    MAX,
};

enum state : uint8_t
{
    PRESSED = 1 << 0,
    RELEASED = 1 << 1,
    HELD = 1 << 2,

    // INVALID = 1 << 7,
};
} // namespace platform::button

namespace platform::event
{

struct pointer
{
    vec::fvec2 begin;
    vec::fvec2 end;
    vec::fvec2 delta;
};

struct analog
{
    vec::fvec2 delta;
    int joystick; // -1 for mouse, otherwise joystick index
};

struct button
{
    enum platform::button::id id;
    enum platform::button::state state;
};

struct window_resize
{
    float aspect_ratio;
    vec::vec2<unsigned int> dimensions;
};

using variant = std::variant<pointer, analog, button, window_resize>;

} // namespace platform::event

namespace platform::frame
{

struct time
{
    double last;
    double now;
    double delta;
};

struct mouse
{
    vec::fvec2 begin;
    vec::fvec2 end;
    vec::fvec2 delta;
    bool is_captured;
};

struct window
{
    float aspect_ratio;
    vec::vec2<unsigned int> dimensions;
};

class state
{
    std::vector<platform::button::id> edge_list;

  public:
    struct time time;
    struct mouse mouse;
    struct window window;
    std::vector<uint8_t> buttons;
    std::vector<platform::event::variant> events;
    bool should_close = false;
    void mouse_warp(const vec::fvec2 &position);
    void mouse_delta(const vec::fvec2 &delta);
    void button_press(enum platform::button::id id);
    void button_release(enum platform::button::id id);
    void clear_edge();
    void clear();
    state() : buttons(platform::button::MAX, 0) {}
};

} // namespace platform::frame

namespace platform::internal
{
class window;
}

namespace platform
{
class window
{
    std::unique_ptr<internal::window> handler;

  public:
    platform::frame::state get_frame();
    window(const std::string &title);
    ~window();
};
} // namespace platform