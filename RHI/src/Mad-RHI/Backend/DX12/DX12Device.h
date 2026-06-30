#pragma once

#include "Mad-RHI/Device.h"
#include "Mad-RHI/Backend/DX12/DX12Factory.h"
#include <directx/d3dx12.h>
#include "Mad-RHI/Backend/DX12/DX12CommandQueue.h"
#include <D3D12MemAlloc.h>
#include "Mad-RHI/Backend/DX12/DX12Resource.h"

namespace mad::rhi {

class DX12Device : public ObjectBase<Device>
{
protected:
    ~DX12Device();

public:
    DX12Device(const DeviceDesc& desc, DX12Factory* factory);

    virtual void EndFrame() override;

    virtual CommandQueue* GetCommandQueue(uint32_t index) override;

    virtual void CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window, CommandQueue* queue) override;
    virtual void CreateTexture(Texture** ppTex, const TextureDesc& desc) override;
    virtual void CreateBuffer(Buffer** ppBuff, const BufferDesc& desc) override;
    virtual void CreateSampler(Sampler** ppSampler, const SamplerDesc& desc) override;
    virtual void CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size) override;
    virtual void CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc) override;
    virtual void CreateFence(Fence** ppFence) override;

    DX12Factory* GetFactory() { return m_Factory; }
    ID3D12Device* GetDevice() { return m_Device; }
    D3D12MA::Allocator* GetAllocator() { return m_Allocator; }

private:
    DX12Factory* m_Factory = nullptr;
    
    ID3D12Device* m_Device = nullptr;
    ID3D12InfoQueue1* m_DebugInfoQueue = nullptr;
    DWORD m_CallbackCookie = 0;

    std::vector<DX12CommandQueue*> m_CommandQueues;

    D3D12MA::Allocator* m_Allocator = nullptr;

};

}