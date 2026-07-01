#include "Mad-RHI/Backend/DX12/Dx/CommandListPool.h"

namespace mad::rhi::dx12 {

void CommandListPool::Init(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
{
    m_Device = device;
    m_Type = type;
}

void CommandListPool::Shutdown()
{
    Purge(UINT64_MAX);

    while (!m_AcquireQueue.empty())
    {
        ID3D12CommandAllocator* allocator = m_AcquireQueue.front();
        m_AcquireQueue.pop_front();
        if (allocator)
        {
            allocator->Release();
        }
    }
}

ID3D12CommandAllocator* CommandListPool::AcquireCommandAllocator()
{
    if (!m_AcquireQueue.empty())
    {
        ID3D12CommandAllocator* allocator = m_AcquireQueue.front();
        m_AcquireQueue.pop_front();
        allocator->Reset();
        return allocator;
    }

    ID3D12CommandAllocator* allocator = nullptr;
    
    m_Device->CreateCommandAllocator(m_Type, IID_PPV_ARGS(&allocator));

    return allocator;
}

void CommandListPool::ReleaseCommandAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue)
{
    m_ReleaseQueue.push_back({ allocator, fenceValue });
}

void CommandListPool::Purge(uint64_t fenceValue)
{
    while (!m_ReleaseQueue.empty())
    {
        auto& entry = m_ReleaseQueue.front();
        if (entry.FenceValue > fenceValue) break;
        m_AcquireQueue.push_back(entry.CommandAllocator);
        m_ReleaseQueue.pop_front();
    }
}
    
}