#pragma once

#include "Mad-RHI/CommandQueue.h"

namespace mad::rhi {

class DX12CommandQueue : public ObjectBase<CommandQueue>
{
protected:
    ~DX12CommandQueue();

public:
    DX12CommandQueue();

    virtual void ResourceBarrier(
        std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers) override;

    virtual void SetRenderTargets(std::vector<TextureView*> colorViews, TextureView* depthView) override;
    virtual void ClearRenderTarget(TextureView* view, const float color[4]) override;
    virtual void ClearDepthStencil(TextureView* view, float depth, uint8_t stencil) override;

    virtual void SetViewport(const Viewport& viewport) override;
    virtual void SetScissorRect(const ScissorRect& scissorRect) override;

    virtual void SetGraphicsPipeline(GraphicsPipelineState* pipeline) override;

    virtual void SetVertexBuffers(uint32_t startSlot, std::vector<Buffer*> buffers, std::vector<uint64_t> offsets) override;
    virtual void SetIndexBuffer(Buffer* buffer, uint64_t byteOffset = 0) override;

    virtual void SetUniformBuffer(const char* name, Buffer* buffer) override;
    virtual void SetStorageBuffer(const char* name, Buffer* buffer) override;
    virtual void SetSampledTexture(const char* name, TextureView* view, Sampler* sampler) override;
    virtual void SetTexture(const char* name, TextureView* view) override;
    virtual void SetSampler(const char* name, Sampler* sampler) override;

    virtual void Draw(uint32_t numVertices, uint32_t firstVertex = 0, 
        uint32_t numInstances = 1, uint32_t firstInstance = 0) override;
    virtual void DrawIndexed(uint32_t numIndices, IndexType indexType, uint32_t firstIndex = 0,
        int32_t baseVertex = 0, uint32_t numInstances = 1, uint32_t firstInstance = 0) override;

    virtual void UpdateTexture(Texture* texture, const void* data, uint64_t size) override;
    virtual void UpdateBuffer(Buffer* buffer, const void* data, uint64_t size) override;
    
    virtual void EnqueueSignal(Fence* fence, uint64_t value) override;
    virtual void WaitForFence(Fence* fence, uint64_t value) override;

    virtual void Flush() override;

private:

};

}