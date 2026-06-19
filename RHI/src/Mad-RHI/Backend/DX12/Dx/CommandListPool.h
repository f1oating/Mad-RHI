#pragma once

#include <deque>
#include <directx/d3d12.h>
#include <utility>

namespace mad::rhi::dx12 {

class CommandListPool
{
struct Entry
{
    ID3D12CommandAllocator* CommandAllocator;
    ID3D12GraphicsCommandList* CommandBuffer;
};

public:
    void Init(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
    void Shutdown();

    Entry AcquireCommandBuffer(ID3D12PipelineState* pipelineState);
    void ReleaseCommandBuffer(Entry entry, uint64_t fenceValue);

    void Purge(uint64_t fenceValue);

private:
    ID3D12Device* m_Device = nullptr;
    D3D12_COMMAND_LIST_TYPE m_Type;

    std::deque<Entry> m_AcquireQueue;
    std::deque<std::pair<uint64_t, Entry>> m_ReleaseQueue;

};

}