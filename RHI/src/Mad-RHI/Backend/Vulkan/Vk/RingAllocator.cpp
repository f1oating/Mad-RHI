#include "Mad-RHI/Backend/Vulkan/Vk/RingAllocator.h"

namespace mad::rhi::vk {

void RingBuffer::Init(VmaAllocator allocator, VkDeviceSize capacity)
{
    m_Allocator = allocator;
    m_Capacity = capacity;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VkBufferCreateInfo bci{};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = m_Capacity;
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vmaCreateBuffer(m_Allocator, &bci, &aci,
        &m_Buffer, &m_VmaAlloc, nullptr);
    vmaMapMemory(m_Allocator, m_VmaAlloc, &m_MappedPtr);
}

void RingBuffer::Shutdown()
{
    if (m_Buffer && m_VmaAlloc)
    {
        vmaUnmapMemory(m_Allocator, m_VmaAlloc);
        vmaDestroyBuffer(m_Allocator, m_Buffer, m_VmaAlloc);
    }
}

Allocation RingBuffer::Allocate(VkDeviceSize size, VkDeviceSize alignment)
{
    VkDeviceSize alignedHead = AlignUp(m_Head, alignment);

    if (alignedHead + size > m_Capacity) {
        alignedHead = 0;
    }

    Allocation alloc;
    alloc.Offset = alignedHead;
    alloc.Size   = size;
    alloc.Mapped = (uint8_t*)m_MappedPtr + alignedHead;

    m_Head = alignedHead + size;
    return alloc;
}

}