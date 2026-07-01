#include "Mad-RHI/Backend/DX12/DX12CommandQueue.h"
#include <iostream>
#include "Mad-RHI/Backend/DX12/DX12Device.h"

namespace mad::rhi {

DX12CommandQueue::DX12CommandQueue(const CommandQueueDesc& desc, DX12Device* context)
{
    m_Context = context;

    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    switch (desc.Type)
    {
    case CommandQueueType::COMMAND_QUEUE_TYPE_GRAPHICS:
    {
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        break;
    }
    case CommandQueueType::COMMAND_QUEUE_TYPE_COMPUTE:
    {
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        break;
    }
    case CommandQueueType::COMMAND_QUEUE_TYPE_TRANSFER:
    {
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        break;
    }
    default:
    {
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        break;
    }
    }

    m_Context->GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Queue));

    m_Fence = new DX12Fence(m_Context);
    
    m_CommandListPool.Init(m_Context->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_CommandAllocator = m_CommandListPool.AcquireCommandAllocator();

    m_Context->GetDevice()->CreateCommandList1(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)
    );
    m_CommandList->Reset(m_CommandAllocator, nullptr);

    std::cout << "DX12CommandQueue Created" << std::endl;
}

DX12CommandQueue::~DX12CommandQueue()
{
    m_Fence->Wait(m_Fence->GetCurrentValue());

    m_CommandListPool.Shutdown();

    if (m_Fence)
    {
        m_Fence->Release();
    }
    if (m_Queue)
    {
        m_Queue->Release();
    }
    
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
    m_CommandList->Close();
    ID3D12CommandList* lists[] = { m_CommandList };
    m_Queue->ExecuteCommandLists(1, lists);
    m_Queue->Signal(m_Fence->GetFence(), m_Fence->IncrementCurrentValue());

    m_CommandListPool.ReleaseCommandAllocator(m_CommandAllocator, m_Fence->GetCurrentValue());
    m_CommandAllocator = m_CommandListPool.AcquireCommandAllocator();
    m_CommandList->Reset(m_CommandAllocator, nullptr);
}

}