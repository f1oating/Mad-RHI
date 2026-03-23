#pragma once

#include <SDL3/SDL.h>

namespace mad::common {

class Window
{
public:
    Window(const char* name, int width, int height);
    ~Window();

    void Update();

    bool IsRunning() { return m_IsRunning; }

private:
    bool m_IsRunning = false;
    SDL_Window* m_Window = nullptr;

};

}