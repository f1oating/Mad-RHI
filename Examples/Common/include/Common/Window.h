#pragma once

#include <SDL3/SDL.h>
#include <xcb/xcb.h>

namespace mad::common {

struct WindowInfo
{
    xcb_connection_t* Connection;
    xcb_window_t Window;
};

class Window
{
public:
    Window(const char* name, int width, int height);
    ~Window();

    void Update();

    void SetRelativeMode(bool enabled);

    bool IsRunning() { return m_IsRunning; }

    WindowInfo GetWindowInfo();

private:
    bool m_IsRunning = false;
    SDL_Window* m_Window = nullptr;

};

}