#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/CommandList.h"
#include "Mad-RHI/PipelineState.h"
#include "Mad-RHI/Resource.h"
#include "Mad-RHI/Fence.h"
#include "Mad-RHI/Swapchain.h"

namespace mad::rhi {
    
struct DeviceDesc
{
    
};

class Device : public Object
{
public:
    virtual ~Device() = default;

    virtual void EndFrame() = 0;
    virtual void GarbageCollect() = 0;

    virtual RefPtr<ImmidiateCommandList> GetImmidiateCommandList() = 0;

    virtual void CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window) = 0;
    virtual void CreateTexture(Texture** ppTex, const TextureDesc& desc) = 0;
    virtual void CreateBuffer(Buffer** ppBuff, const BufferDesc& desc) = 0;
    virtual void CreateSampler(Sampler** ppSampler, const SamplerDesc& desc) = 0;
    virtual void CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size) = 0;
    virtual void CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc) = 0;
    virtual void CreateFence(Fence** ppFence) = 0;

};

}