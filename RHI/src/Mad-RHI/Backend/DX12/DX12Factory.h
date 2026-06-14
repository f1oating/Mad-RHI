#pragma once

#include "Mad-RHI/Factory.h"
#include <vector>

namespace mad::rhi {

class DX12Factory : public Factory
{
public:
    DX12Factory(const FactoryInitInfo& info);
    ~DX12Factory();

    virtual void EnumerateAdapters(uint32_t& numAdapters, AdapterInfo* adapters) override;
    
    virtual void CreateDevice(Device** ppDevice, const DeviceDesc& desc) override;

private:

};

}