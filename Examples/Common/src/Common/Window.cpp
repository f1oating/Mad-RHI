#include "Common/Window.h"
#include "Common/Event.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <xcb/xcb.h>
#endif

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
        if (event.type == SDL_EVENT_WINDOW_RESIZED)
        {
            WindowResizeEvent wre{};
            wre.Width = event.window.data1;
            wre.Height = event.window.data2;
            EventBus::Emit(wre);
        }
    }
}

void Window::SetRelativeMode(bool enabled)
{
    SDL_SetWindowRelativeMouseMode(m_Window, enabled);
}

rhi::WindowHandle Window::GetWindowInfo()
{
    rhi::WindowHandle handle;

    #ifdef _WIN32
        HWND hwnd = (HWND)SDL_GetPointerProperty(
            SDL_GetWindowProperties(m_Window),
            SDL_PROP_WINDOW_WIN32_HWND_POINTER,
            nullptr
        );

        HINSTANCE hinstance = (HINSTANCE)SDL_GetPointerProperty(
            SDL_GetWindowProperties(m_Window),
            SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER,
            nullptr
        );

        handle.platform = rhi::WindowHandle::Platform::WIN;
        handle.win32.hwnd = hwnd;
        handle.win32.hinstance = hinstance;
    #else
        Display* xDisplay = (Display*)SDL_GetPointerProperty(
            SDL_GetWindowProperties(m_Window),
            SDL_PROP_WINDOW_X11_DISPLAY_POINTER,
            nullptr
        );

        xcb_window_t xcbWindow = (xcb_window_t)SDL_GetNumberProperty(
            SDL_GetWindowProperties(m_Window),
            SDL_PROP_WINDOW_X11_WINDOW_NUMBER,
            0
        );

        xcb_connection_t* connection = XGetXCBConnection(xDisplay);

        handle.platform = rhi::WindowHandle::Platform::XCB;
        handle.xcb.connection = connection;
        handle.xcb.window = xcbWindow;
    #endif

    return handle;
}

}