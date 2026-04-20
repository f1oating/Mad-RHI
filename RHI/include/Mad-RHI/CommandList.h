#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/Resource.h"
#include "Mad-RHI/Fence.h"
#include "Mad-RHI/PipelineState.h"
#include <cstdint>
#include <vector>

namespace mad::rhi {

class ImmidiateCommandList : public Object
{
public:
    virtual ~ImmidiateCommandList() = default;

    virtual void ResourceBarrier(
        std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers) = 0;

    virtual void SetRenderTargets(std::vector<TextureView*> colorViews, TextureView* depthView) = 0;
    virtual void ClearRenderTarget(TextureView* view, const float color[4]) = 0;
    virtual void ClearDepthStencil(TextureView* view, float depth, uint8_t stencil) = 0;    
    
    virtual void SetGraphicsPipeline(GraphicsPipelineState* pipeline) = 0;

    virtual void SetVertexBuffers(uint32_t startSlot, std::vector<Buffer*> buffers, std::vector<uint64_t> offsets) = 0;

    virtual void Draw(uint32_t numVertices, uint32_t firstVertex) = 0;

    virtual void UpdateTexture(Texture* texture, const void* data, uint64_t size) = 0;
    virtual void UpdateBuffer(Buffer* buffer, const void* data, uint64_t size) = 0;

    virtual void EnqueueSignal(Fence* fence, uint64_t value) = 0;
    virtual void WaitForFence(Fence* fence, uint64_t value) = 0;

    virtual void Flush() = 0;

};

}