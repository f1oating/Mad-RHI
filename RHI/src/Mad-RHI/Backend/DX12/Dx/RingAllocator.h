#pragma once

#include <cstdint>
#include <deque>
#include <directx/d3d12.h>
#include <D3D12MemAlloc.h>

namespace mad::rhi::dx12 {

inline uint64_t AlignUp(uint64_t value, uint64_t alignment) 
{
    return (value + alignment - 1) & ~(alignment - 1);
}

struct Allocation 
{
    uint64_t Offset;
    uint64_t Size;
    void* Mapped;
};

class RingBuffer
{
public:
    void Init(D3D12MA::Allocator* allocator, uint64_t capacity = 1 * 1024 * 1024);
    void Shutdown();

    Allocation Allocate(uint64_t size, uint64_t alignment);

    uint64_t GetHead() { return m_Head; }
    ID3D12Resource* GetBuffer() { return m_Buffer; }
    void* GetMappedPtr() { return m_MappedPtr; }

    void SetTail(uint64_t tail) { m_Tail = tail; }

private:
    D3D12MA::Allocator* m_Allocator = nullptr;
    D3D12MA::Allocation* m_Allocation = nullptr;
    ID3D12Resource* m_Buffer = nullptr;
    void* m_MappedPtr = nullptr;

    uint64_t m_Capacity = 0;
    uint64_t m_Head = 0;
    uint64_t m_Tail = 0;

};

}