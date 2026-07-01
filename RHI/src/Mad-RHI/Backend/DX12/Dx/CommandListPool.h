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
    uint64_t FenceValue;
};

public:
    void Init(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
    void Shutdown();

    ID3D12CommandAllocator* AcquireCommandAllocator();
    void ReleaseCommandAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue);

    void Purge(uint64_t fenceValue);

private:
    ID3D12Device* m_Device = nullptr;
    D3D12_COMMAND_LIST_TYPE m_Type;

    std::deque<ID3D12CommandAllocator*> m_AcquireQueue;
    std::deque<Entry> m_ReleaseQueue;

};

}