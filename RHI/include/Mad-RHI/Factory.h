#pragma once

#include "Mad-RHI/Device.h"

namespace mad::rhi {

enum class FactoryBackend
{
    Vulkan, DX12
};

struct FactoryInitInfo
{
    FactoryBackend Backend;
    const char* pAppName;
    const char* pEngineName;
};

struct AdapterInfo
{
    char Description[256];
    uint32_t VendorId;
    uint32_t DeviceId;
};

class MAD_RHI_API Factory
{
protected:
    static Factory* s_Factory;
    static bool s_IsInitialized;

public:
    virtual ~Factory() = default;

    static void Init(const FactoryInitInfo& info);
    static void Shutdown();

    virtual void EnumerateAdapters(uint32_t& numAdapters, AdapterInfo* adapters) = 0;

    virtual void CreateDevice(Device** ppDevice, const DeviceDesc& desc) = 0;

    static Factory* Get();

};

}