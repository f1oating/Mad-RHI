#pragma once

#include "Mad-RHI/Device.h"

namespace mad::rhi {

enum class FactoryBackend
{
    Vulkan
};

struct FactoryInitInfo
{
    FactoryBackend Backend;
    const char* pAppName;
    const char* pEngineName;
};

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

class Factory
{
protected:
    static Factory* s_Factory;
    static bool s_IsInitialized;

public:
    virtual ~Factory() = default;

    static void Init(const FactoryInitInfo& info);
    static void Shutdown();

    virtual void CreateDevice(Device** ppDevice, const WindowHandle& wh) = 0;

    static Factory* GetInstance();

};

}