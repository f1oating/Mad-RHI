#include "Common/Window.h"

namespace mad::common {

Window::Window(const char* name, int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO);

    m_Window = SDL_CreateWindow(
        name, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    m_IsRunning = true;
}

Window::~Window()
{
    SDL_DestroyWindow(m_Window);
    SDL_Quit();
}

void Window::Update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT) m_IsRunning = false;
    }
}

}