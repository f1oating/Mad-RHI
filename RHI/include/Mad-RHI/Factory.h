#pragma once

#include "Mad-RHI/Device.h"
#include "Mad-RHI/CommandList.h"
#include <xcb/xcb.h>

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

class Factory
{
protected:
    static Factory* s_Factory;
    static bool s_IsInitialized;

public:
    virtual ~Factory() = default;

    static void Init(FactoryInitInfo& info);
    static void Shutdown();

    virtual void CreateDevice(Device** ppDevice, ImmidiateCommandList** ppIcl, xcb_connection_t* connection, xcb_window_t window) = 0;

    static Factory* GetInstance();

};

}