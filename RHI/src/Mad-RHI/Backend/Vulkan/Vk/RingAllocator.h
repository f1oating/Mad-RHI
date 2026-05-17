#pragma once

#include <cstdint>
#include <volk/volk.h>
#include <deque>
#include <vk_mem_alloc.h>

namespace mad::rhi::vk {

inline VkDeviceSize AlignUp(VkDeviceSize value, VkDeviceSize alignment) 
{
    return (value + alignment - 1) & ~(alignment - 1);
}

struct Allocation 
{
    VkDeviceSize Offset;
    VkDeviceSize Size;
    void* Mapped;
};

class RingBuffer
{
public:
    void Init(VmaAllocator allocator, VkDeviceSize capacity = 1 * 1024 * 1024);
    void Shutdown();

    Allocation Allocate(VkDeviceSize size, VkDeviceSize alignment);

    VkDeviceSize GetHead() { return m_Head; }
    VkBuffer GetBuffer() { return m_Buffer; }
    void* GetMappedPtr() { return m_MappedPtr; }

    void SetTail(VkDeviceSize tail) { m_Tail = tail; }

private:
    VmaAllocator m_Allocator = nullptr;
    VmaAllocation m_VmaAlloc = nullptr;
    VkBuffer m_Buffer = nullptr;
    void* m_MappedPtr = nullptr;

    VkDeviceSize m_Capacity = 0;
    VkDeviceSize m_Head = 0;
    VkDeviceSize m_Tail = 0;

};

}