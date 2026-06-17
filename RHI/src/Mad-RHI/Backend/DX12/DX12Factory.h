#pragma once

#include "Mad-RHI/Factory.h"
#include <vector>
#include <d3dx12/d3dx12.h>
#include <dxgi1_2.h>

namespace mad::rhi {

class DX12Factory : public Factory
{
public:
    DX12Factory(const FactoryInitInfo& info);
    ~DX12Factory();

    virtual void EnumerateAdapters(uint32_t& numAdapters, AdapterInfo* adapters) override;
    
    virtual void CreateDevice(Device** ppDevice, const DeviceDesc& desc) override;

    IDXGIFactory2* GetFactory() { return m_Factory; }
    IDXGIAdapter* GetAdapter(uint32_t index) { return m_Adapters[index]; }

private:
    ID3D12Debug3* m_DebugController = nullptr;
    ID3D12InfoQueue* m_DebugInfoQueue = nullptr;
    IDXGIFactory2* m_Factory = nullptr;

    std::vector<IDXGIAdapter*> m_Adapters;

};

}