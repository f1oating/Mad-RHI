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
        Entry entry = m_AcquireQueue.front();
        m_AcquireQueue.pop_front();
        if (entry.CommandBuffer)
        {
            entry.CommandBuffer->Release();
        }
        if (entry.CommandAllocator)
        {
            entry.CommandAllocator->Release();
        }
    }
}

CommandListPool::Entry CommandListPool::AcquireCommandBuffer(ID3D12PipelineState* pipelineState)
{
    if (!m_AcquireQueue.empty())
    {
        Entry entry = m_AcquireQueue.front();
        m_AcquireQueue.pop_front();
        entry.CommandAllocator->Reset();
        entry.CommandBuffer->Reset(entry.CommandAllocator, pipelineState);
        return entry;
    }

    Entry entry {};
    
    m_Device->CreateCommandAllocator(m_Type, IID_PPV_ARGS(&entry.CommandAllocator));
    m_Device->CreateCommandList(0, m_Type, entry.CommandAllocator, pipelineState, IID_PPV_ARGS(&entry.CommandBuffer));

    return entry;
}

void CommandListPool::ReleaseCommandBuffer(Entry entry, uint64_t fenceValue)
{
    m_ReleaseQueue.push_back({ fenceValue, entry });
}

void CommandListPool::Purge(uint64_t fenceValue)
{
    while (!m_ReleaseQueue.empty())
    {
        auto& pair = m_ReleaseQueue.front();
        if (pair.first > fenceValue) break;
        m_AcquireQueue.push_back(pair.second);
        m_ReleaseQueue.pop_front();
    }
}
    
}