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

class Factory
{
protected:
    static Factory* s_Factory;
    static bool s_IsInitialized;

public:
    virtual ~Factory() = default;

    static void Init(const FactoryInitInfo& info);
    static void Shutdown();

    virtual void CreateDevice(Device** ppDevice, const DeviceDesc& desc) = 0;

    static Factory* Get();

};

}