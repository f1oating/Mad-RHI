#include "Mad-RHI/Backend/DX12/DX12Device.h"
#include <iostream>

namespace mad::rhi {

DX12Device::DX12Device(const DeviceDesc& desc, DX12Factory* factory)
{
    m_Factory = factory;

    D3D12CreateDevice(m_Factory->GetAdapter(desc.AdapterId), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));

    std::cout << "DX12Device Created" << std::endl;

    m_CommandQueues.resize(desc.NumCommandQueues);
    for (int i = 0; i < desc.NumCommandQueues; i++)
    {
        m_CommandQueues[i] = new DX12CommandQueue(desc.pCommandQueues[i], this);
    }   
}

DX12Device::~DX12Device()
{
    for (auto* queue : m_CommandQueues)
    {
        if (queue)
        {
            queue->Release();
        }
    }
    m_CommandQueues.clear();
    
    if (m_Device)
    {
        m_Device->Release();
    }

    std::cout << "DX12Device Destroyed" << std::endl;
}

void DX12Device::EndFrame()
{

}

CommandQueue* DX12Device::GetCommandQueue(uint32_t index)
{
    return m_CommandQueues[index];
}

void DX12Device::CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window, CommandQueue* queue)
{

}

void DX12Device::CreateTexture(Texture** ppTex, const TextureDesc& desc)
{

}

void DX12Device::CreateBuffer(Buffer** ppBuff, const BufferDesc& desc)
{

}

void DX12Device::CreateSampler(Sampler** ppSampler, const SamplerDesc& desc)
{

}

void DX12Device::CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size)
{

}

void DX12Device::CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc)
{

}

void DX12Device::CreateFence(Fence** ppFence)
{

}

}