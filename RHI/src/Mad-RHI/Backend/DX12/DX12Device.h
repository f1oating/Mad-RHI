#pragma once

#include "Mad-RHI/Device.h"

namespace mad::rhi {

class DX12Device : public ObjectBase<Device>
{
protected:
    ~DX12Device();

public:
    DX12Device(const DeviceDesc& desc);

    virtual void EndFrame() override;

    virtual CommandQueue* GetCommandQueue(uint32_t index) override;

    virtual void CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window, CommandQueue* queue) override;
    virtual void CreateTexture(Texture** ppTex, const TextureDesc& desc) override;
    virtual void CreateBuffer(Buffer** ppBuff, const BufferDesc& desc) override;
    virtual void CreateSampler(Sampler** ppSampler, const SamplerDesc& desc) override;
    virtual void CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size) override;
    virtual void CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc) override;
    virtual void CreateFence(Fence** ppFence) override;

private:

};

}