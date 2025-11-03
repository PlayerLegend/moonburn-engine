#include <SDL2/SDL.h>
#include <assert.h>
#include <engine/platform.hpp>
#include <glad/glad.h>
#include <iostream>
#include <stdlib.h>

class sdl_handler
{

  public:
    platform::frame::state frame;

    sdl_handler()
    {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
            exit(1);
        }
    }
    ~sdl_handler()
    {
        SDL_Quit();
    }
} sdl_handler;

namespace platform::internal
{
class window
{
  public:
    SDL_Window *sdl_window = NULL;
    SDL_GLContext sdl_gl_context = NULL;
    platform::frame::state frame;

    window(std::string title)
    {

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);

        sdl_window = SDL_CreateWindow(title.c_str(),
                                      SDL_WINDOWPOS_UNDEFINED,
                                      SDL_WINDOWPOS_UNDEFINED,
                                      800,
                                      600,
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
                                          SDL_WINDOW_RESIZABLE);

        if (!sdl_window)
        {
            std::cout << "SDL_CreateWindow Error: " << SDL_GetError()
                      << std::endl;

            return;
        }

        sdl_gl_context = SDL_GL_CreateContext(sdl_window);

        if (!sdl_gl_context)
        {
            std::cout << "SDL_GL_CreateContext Error: " << SDL_GetError()
                      << std::endl;
            return;
        }

        if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            exit(1);
        }
    }

    ~window()
    {
        if (sdl_gl_context)
        {
            SDL_GL_DeleteContext(sdl_gl_context);
        }

        if (sdl_window)
        {
            SDL_DestroyWindow(sdl_window);
        }
    }
};
} // namespace platform::internal

namespace platform::button
{
static enum id id_from_sdl_mouse_button(uint8_t button)
{
    switch (button)
    {
    case SDL_BUTTON_LEFT:
        return id::MOUSE1;
    case SDL_BUTTON_RIGHT:
        return id::MOUSE2;
    case SDL_BUTTON_MIDDLE:
        return id::MOUSE3;
    default:
        return id::INVALID;
    }
}
static enum id id_from_sdl_scancode(SDL_Scancode scancode)
{
#define map2(from, to)                                                         \
    case SDL_SCANCODE_##from:                                                  \
        return id::to;
#define map(key) map2(key, key)

    switch (scancode)
    {
        map(A);
        map(B);
        map(C);
        map(D);
        map(E);
        map(F);
        map(G);
        map(H);
        map(I);
        map(J);
        map(K);
        map(L);
        map(M);
        map(N);
        map(O);
        map(P);
        map(Q);
        map(R);
        map(S);
        map(T);
        map(U);
        map(V);
        map(W);
        map(X);
        map(Y);
        map(Z);

        map2(0, N0);
        map2(1, N1);
        map2(2, N2);
        map2(3, N3);
        map2(4, N4);
        map2(5, N5);
        map2(6, N6);
        map2(7, N7);
        map2(8, N8);
        map2(9, N9);

        map2(RETURN, ENTER);
        map(ESCAPE);
        map(BACKSPACE);
        map(TAB);
        map(SPACE);
        map(MINUS);

        map2(EQUALS, EQUAL);
        map2(LEFTBRACKET, LEFT_BRACKET);
        map2(RIGHTBRACKET, RIGHT_BRACKET);
        map(BACKSLASH);
        map2(NONUSHASH, BACKSLASH);
        map(SEMICOLON);
        map(APOSTROPHE);
        map(GRAVE);
        map(COMMA);
        map(PERIOD);
        map(SLASH);
        map(CAPSLOCK);

        map(F1);
        map(F2);
        map(F3);
        map(F4);
        map(F5);
        map(F6);
        map(F7);
        map(F8);
        map(F9);
        map(F10);
        map(F11);
        map(F12);
        map2(LSHIFT, LEFT_SHIFT);
        map2(RSHIFT, LEFT_SHIFT);
        map2(LALT, LEFT_ALT);
        map2(RALT, LEFT_ALT);
        map2(LCTRL, LEFT_CONTROL);
        map2(RCTRL, LEFT_CONTROL);

    default:
        return id::INVALID;
    }

#undef map
#undef map2
}

} // namespace platform::button

platform::window::window(const std::string &title)
    : handler(std::make_unique<internal::window>(title))
{
}

platform::window::~window() = default;

static void button_event(platform::frame::state &frame, SDL_KeyboardEvent event)
{
    if (event.repeat)
    {
        return;
    }

    enum platform::button::id id =
        platform::button::id_from_sdl_scancode(event.keysym.scancode);

    if (event.state == SDL_PRESSED)
    {
        frame.button_press(id);
    }
    else
    {
        assert(event.state == SDL_RELEASED);
        frame.button_release(id);
    }
}

static void mouse_motion_event(platform::frame::state &frame,
                               SDL_MouseMotionEvent event)
{
    vec::fvec2 end = vec::fvec2(event.x, event.y);
    frame.mouse_warp(end);
}

static void window_event(platform::frame::state &frame, SDL_WindowEvent event)
{
    if (event.event == SDL_WINDOWEVENT_CLOSE)
    {
        frame.should_close = true;
    }
    else if (event.event == SDL_WINDOWEVENT_RESIZED)
    {
        frame.window_resize(
            vec::vec2<unsigned int>{static_cast<unsigned int>(event.data1),
                                    static_cast<unsigned int>(event.data2)});
    }
}

static void mouse_button_event(platform::frame::state &frame,
                               SDL_MouseButtonEvent event)
{
    enum platform::button::id id =
        platform::button::id_from_sdl_mouse_button(event.button);

    if (event.state == SDL_PRESSED)
    {
        frame.button_press(id);
    }
    else
    {
        assert(event.state == SDL_RELEASED);
        frame.button_release(id);
    }
}

static void mouse_wheel_event(platform::frame::state &frame,
                              SDL_MouseWheelEvent event)
{
    if (event.y > 0)
    {
        frame.button_press(platform::button::MOUSEWHEEL_UP);
        frame.button_release(platform::button::MOUSEWHEEL_UP);
    }
    else if (event.y < 0)
    {
        frame.button_press(platform::button::MOUSEWHEEL_DOWN);
        frame.button_release(platform::button::MOUSEWHEEL_DOWN);
    }
}

platform::frame::state platform::window::get_frame()
{
    platform::frame::state &frame = handler->frame;

    if (frame.should_close)
    {
        return frame;
    }

    frame.clear();

    SDL_GL_SwapWindow(handler->sdl_window);

    glViewport(0, 0, frame.window.dimensions.x, frame.window.dimensions.y);

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    SDL_Event sdl_event;

    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type)
        {
        case SDL_MOUSEMOTION:
            mouse_motion_event(frame, sdl_event.motion);
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            button_event(frame, sdl_event.key);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            mouse_button_event(frame, sdl_event.button);
            break;
        case SDL_WINDOWEVENT:
            window_event(frame, sdl_event.window);
            break;
        case SDL_MOUSEWHEEL:
            mouse_wheel_event(frame, sdl_event.wheel);
            break;
        }
    }
    frame.time.last = frame.time.now;
    uint64_t ticks = SDL_GetTicks64();
    frame.time.now = (double)ticks / (double)1000.0;
    frame.time.delta = frame.time.now - frame.time.last;
    return frame;
}