#include "Mad-RHI/Backend/DX12/Dx/RingAllocator.h"

namespace mad::rhi::dx12 {

void RingBuffer::Init(D3D12MA::Allocator* allocator, uint64_t capacity)
{
    m_Allocator = allocator;
    m_Capacity = capacity;

    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = capacity;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;

    m_Allocator->CreateResource(
        &allocationDesc,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        NULL,
        &m_Allocation,
        IID_NULL, NULL);

    m_Buffer = m_Allocation->GetResource();
}

void RingBuffer::Shutdown()
{
    if (m_Allocation)
    {
        m_Allocation->Release();
    }
}

Allocation RingBuffer::Allocate(uint64_t size, uint64_t alignment)
{
    uint64_t alignedHead = AlignUp(m_Head, alignment);

    if (alignedHead + size > m_Capacity) {
        alignedHead = 0;
    }

    Allocation alloc;
    alloc.Offset = alignedHead;
    alloc.Size = size;
    alloc.Mapped = (uint8_t*)m_MappedPtr + alignedHead;

    m_Head = alignedHead + size;
    return alloc;
}

}