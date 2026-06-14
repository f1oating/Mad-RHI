#include "Mad-RHI/Backend/DX12/DX12Factory.h"
#include <cstdint>

namespace mad::rhi {

DX12Factory::DX12Factory(const FactoryInitInfo& info)
{
    D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&m_DebugController);
    m_DebugController->EnableDebugLayer();

    CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_Factory);
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
}

void DX12Factory::EnumerateAdapters(uint32_t& numAdapters, AdapterInfo* adapters)
{
    IDXGIAdapter* dxAdapter = nullptr;
    numAdapters = 0;
    
    for (uint32_t i = 0; SUCCEEDED(m_Factory->EnumAdapters(i, &dxAdapter)); i++)
    {
        numAdapters++;

        if (adapters)
        {
            DXGI_ADAPTER_DESC dxAdapterDesc {};
            dxAdapter->GetDesc(&dxAdapterDesc);

            AdapterInfo adapterInfo {};
            adapterInfo.DeviceId = dxAdapterDesc.DeviceId;
            adapterInfo.VendorId = dxAdapterDesc.VendorId;

            adapters[i] = adapterInfo;
        }
    }
}

void DX12Factory::CreateDevice(Device** ppDevice, const DeviceDesc& desc)
{
    
}

}