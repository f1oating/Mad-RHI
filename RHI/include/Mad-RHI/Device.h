#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/CommandQueue.h"
#include "Mad-RHI/PipelineState.h"
#include "Mad-RHI/Resource.h"
#include "Mad-RHI/Fence.h"
#include "Mad-RHI/Swapchain.h"

namespace mad::rhi {
    
enum CommandQueueTypeFlags : uint8_t
{
    COMMAND_QUEUE_TYPE_GRAPHICS_BIT = 1 << 0,
    COMMAND_QUEUE_TYPE_COMPUTE_BIT = 1 << 1,
    COMMAND_QUEUE_TYPE_TRANSFER_BIT = 1 << 2,
};

struct QueueFamilyInfo
{
    uint32_t Index;
    CommandQueueTypeFlags Flags;
    uint32_t MaxQueues;
};

struct CommandQueueDesc
{
    uint32_t Index;
    CommandQueueTypeFlags Flags = COMMAND_QUEUE_TYPE_GRAPHICS_BIT;
};

struct DeviceDesc
{
    uint32_t AdapterId = 0;
    const CommandQueueDesc* pCommandQueues = nullptr;
    uint32_t NumCommandQueues = 0;
};

class Device : public Object
{
public:
    virtual ~Device() = default;

    virtual void EndFrame() = 0;
    virtual void GarbageCollect() = 0;

    virtual CommandQueue* GetCommandQueue(uint32_t index) = 0;

    virtual void CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window, CommandQueue* queue) = 0;
    virtual void CreateTexture(Texture** ppTex, const TextureDesc& desc) = 0;
    virtual void CreateBuffer(Buffer** ppBuff, const BufferDesc& desc) = 0;
    virtual void CreateSampler(Sampler** ppSampler, const SamplerDesc& desc) = 0;
    virtual void CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size) = 0;
    virtual void CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc) = 0;
    virtual void CreateFence(Fence** ppFence) = 0;

};

}