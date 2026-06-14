#include "Mad-RHI/Backend/DX12/DX12CommandQueue.h"
#include <iostream>

namespace mad::rhi {

DX12CommandQueue::DX12CommandQueue()
{
    std::cout << "DX12CommandQueue Created" << std::endl;
}

DX12CommandQueue::~DX12CommandQueue()
{
    std::cout << "DX12CommandQueue Destroyed" << std::endl;
}

void DX12CommandQueue::ResourceBarrier(
    std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers)
{
    
}

void DX12CommandQueue::SetRenderTargets(std::vector<TextureView*> colorViews, TextureView* depthView)
{

}

void DX12CommandQueue::ClearRenderTarget(TextureView* view, const float color[4])
{

}

void DX12CommandQueue::ClearDepthStencil(TextureView* view, float depth, uint8_t stencil)
{

}

void DX12CommandQueue::SetViewport(const Viewport& viewport)
{

}

void DX12CommandQueue::SetScissorRect(const ScissorRect& scissorRect)
{

}

void DX12CommandQueue::SetGraphicsPipeline(GraphicsPipelineState* pipeline)
{

}

void DX12CommandQueue::SetVertexBuffers(uint32_t startSlot, std::vector<Buffer*> buffers, std::vector<uint64_t> offsets)
{

}

void DX12CommandQueue::SetIndexBuffer(Buffer* buffer, uint64_t byteOffset)
{

}

void DX12CommandQueue::SetUniformBuffer(const char* name, Buffer* buffer)
{

}

void DX12CommandQueue::SetStorageBuffer(const char* name, Buffer* buffer)
{

}

void DX12CommandQueue::SetSampledTexture(const char* name, TextureView* view, Sampler* sampler)
{

}

void DX12CommandQueue::SetTexture(const char* name, TextureView* view)
{

}

void DX12CommandQueue::SetSampler(const char* name, Sampler* sampler)
{

}

void DX12CommandQueue::Draw(uint32_t numVertices, uint32_t firstVertex, 
    uint32_t numInstances, uint32_t firstInstance)
{

}

void DX12CommandQueue::DrawIndexed(uint32_t numIndices, IndexType indexType, uint32_t firstIndex,
    int32_t baseVertex, uint32_t numInstances, uint32_t firstInstance)
{

}

void DX12CommandQueue::UpdateTexture(Texture* texture, const void* data, uint64_t size)
{

}

void DX12CommandQueue::UpdateBuffer(Buffer* buffer, const void* data, uint64_t size)
{

}

void DX12CommandQueue::EnqueueSignal(Fence* fence, uint64_t value)
{

}

void DX12CommandQueue::WaitForFence(Fence* fence, uint64_t value)
{

}

void DX12CommandQueue::Flush()
{

}

}