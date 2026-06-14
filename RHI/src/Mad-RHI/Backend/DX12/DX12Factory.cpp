#include "Mad-RHI/Backend/DX12/DX12Factory.h"
#include <cstdint>
#include "Mad-RHI/Backend/DX12/DX12Device.h"
#include <iostream>

namespace mad::rhi {

DX12Factory::DX12Factory(const FactoryInitInfo& info)
{
    CreateDXGIFactory(IID_PPV_ARGS(&m_Factory));

    IDXGIAdapter* dxAdapter = nullptr;
    for (uint32_t i = 0; SUCCEEDED(m_Factory->EnumAdapters(i, &dxAdapter)); i++)
    {
        m_Adapters.push_back(dxAdapter);   
    }

    std::cout << "DX12Factory Created" << std::endl;
}

DX12Factory::~DX12Factory()
{
    if (m_Factory)
    {
        m_Factory->Release();
    }
    if (m_DebugController)
    {
        m_DebugController->Release();
    }

    std::cout << "DX12Factory Destroyed" << std::endl;
}

void DX12Factory::EnumerateAdapters(uint32_t& numAdapters, AdapterInfo* adapters)
{
    numAdapters = m_Adapters.size();
    if (!adapters) return;
    
    for (int i = 0; i < numAdapters; i++)
    {
        DXGI_ADAPTER_DESC dxAdapterDesc {};
        m_Adapters[i]->GetDesc(&dxAdapterDesc);

        AdapterInfo adapterInfo {};
        adapterInfo.DeviceId = dxAdapterDesc.DeviceId;
        adapterInfo.VendorId = dxAdapterDesc.VendorId;

        adapters[i] = adapterInfo;
    }
}

void DX12Factory::CreateDevice(Device** ppDevice, const DeviceDesc& desc)
{
    *ppDevice = new DX12Device(desc, this);
}

}