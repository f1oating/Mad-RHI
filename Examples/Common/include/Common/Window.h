#pragma once

#include <SDL3/SDL.h>
#include "Mad-RHI/Swapchain.h"

namespace mad::common {

class Window
{
public:
    Window(const char* name, int width, int height);
    ~Window();

    void Update();

    void SetRelativeMode(bool enabled);

    bool IsRunning() { return m_IsRunning; }

    rhi::WindowHandle GetWindowInfo();

private:
    bool m_IsRunning = false;
    SDL_Window* m_Window = nullptr;

};

}