#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/Resource.h"
#include "Mad-RHI/Fence.h"
#include "Mad-RHI/PipelineState.h"
#include <cstdint>
#include <vector>

namespace mad::rhi {

enum class IndexType 
{ 
    Uint16, Uint32
};

class CommandQueue : public Object
{
public:
    virtual ~CommandQueue() = default;

    virtual void ResourceBarrier(
        std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers) = 0;

    virtual void BeginRendering() = 0;
    virtual void EndRendering() = 0;

    virtual void SetRenderTargets(std::vector<TextureView*> colorViews, TextureView* depthView) = 0;
    virtual void ClearRenderTarget(TextureView* view, const float color[4]) = 0;
    virtual void ClearDepthStencil(TextureView* view, float depth, uint8_t stencil) = 0;    
    
    virtual void SetGraphicsPipeline(GraphicsPipelineState* pipeline) = 0;

    virtual void SetVertexBuffers(uint32_t startSlot, std::vector<Buffer*> buffers, std::vector<uint64_t> offsets) = 0;
    virtual void SetIndexBuffer(Buffer* buffer, uint64_t byteOffset = 0) = 0;

    virtual void SetUniformBuffer(const char* name, Buffer* buffer) = 0;
    virtual void SetStorageBuffer(const char* name, Buffer* buffer) = 0;
    virtual void SetSampledTexture(const char* name, TextureView* view, Sampler* sampler) = 0;
    virtual void SetTexture(const char* name, TextureView* view) = 0;
    virtual void SetSampler(const char* name, Sampler* sampler) = 0;

    virtual void Draw(uint32_t numVertices, uint32_t firstVertex = 0, 
        uint32_t numInstances = 1, uint32_t firstInstance = 0) = 0;
    virtual void DrawIndexed(uint32_t numIndices, IndexType indexType, uint32_t firstIndex = 0,
        int32_t baseVertex = 0, uint32_t numInstances = 1, uint32_t firstInstance = 0) = 0;

    virtual void UpdateTexture(Texture* texture, const void* data, uint64_t size) = 0;
    virtual void UpdateBuffer(Buffer* buffer, const void* data, uint64_t size) = 0;

    virtual void EnqueueSignal(Fence* fence, uint64_t value) = 0;
    virtual void WaitForFence(Fence* fence, uint64_t value) = 0;

    virtual void Flush() = 0;

};

}