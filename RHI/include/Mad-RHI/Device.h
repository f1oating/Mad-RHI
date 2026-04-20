#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/CommandList.h"
#include "Mad-RHI/PipelineState.h"
#include "Mad-RHI/Resource.h"
#include "Mad-RHI/Fence.h"

namespace mad::rhi {
    
struct WindowHandle
{
    enum class Platform { WIN, WAYLAND, XCB };
    Platform platform;

    union
    {
        struct { void* connection; uint32_t window; } xcb;
        struct { void* display; void* surface; } wayland;
        struct { void* hwnd; void* hinstance; } win32;
    };
};

struct DeviceDesc
{
    WindowHandle Window;
};

class Device : public Object
{
public:
    virtual ~Device() = default;

    virtual void Resize() = 0;

    virtual void EndFrame() = 0;
    virtual void GarbageCollect() = 0;
    
    virtual Texture* GetCurrentBackBuffer() = 0;

    virtual void Present() = 0;

    virtual RefPtr<ImmidiateCommandList> GetImmidiateCommandList() = 0;

    virtual void CreateTexture(Texture** ppTex, const TextureDesc& desc) = 0;
    virtual void CreateBuffer(Buffer** ppBuff, const BufferDesc& desc) = 0;
    virtual void CreateSampler(Sampler** ppSampler, const SamplerDesc& desc) = 0;
    virtual void CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size) = 0;
    virtual void CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc) = 0;
    virtual void CreateFence(Fence** ppFence) = 0;

};

}