#pragma once

#include "Mad-RHI/Common.h"
#include <cstdint>
#include "Mad-RHI/Resource.h"

namespace mad::rhi {
    
struct WindowHandle
{
    enum class Platform { WIN, WAYLAND, XCB };
    Platform platform;

    union
    {
        struct { void* connection; uint32_t window; } xcb;
        struct { void* display; void* surface; } wayland;
        struct { void* hwnd; void* hinstance; } win32;
    };
};

class Swapchain : public Object
{
public:
    virtual ~Swapchain() = default;

    virtual Texture* GetCurrentBackBuffer() = 0;
    virtual void Present() = 0;

};

}