#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

namespace mad::rhi {

VulkanTexture::VulkanTexture(const TextureDesc& desc, VkImage image)
{
    m_Desc = desc;
    m_Image = image;
}

VulkanTexture::VulkanTexture(const TextureDesc& desc, VkImage image, 
    VmaAllocation allocation, VmaAllocator allocator, VulkanDevice* context)
{
    m_Desc = desc;
    m_Image = image;
    m_Allocation = allocation;
    m_Allocator = allocator;
    m_Context = context;
}

VulkanTexture::~VulkanTexture()
{
    if (m_Image && m_Allocation)
    {
        m_Context->SafeReleaseResource(new VkImageResource{ m_Image, m_Allocation, m_Allocator });
    }
}

ResourceState VulkanTexture::GetCurrentResourceState()
{
    return m_CurrentState;
}

VulkanBuffer::VulkanBuffer(const BufferDesc& desc, VkBuffer buffer, void* mapped, VulkanDevice* context)
{
    m_Desc = desc;
    m_Buffer = buffer;
    m_MappedPtr = mapped;
    m_Context = context;
}

VulkanBuffer::VulkanBuffer(const BufferDesc& desc, VkBuffer buffer, VmaAllocation allocation, 
    VmaAllocator allocator, VulkanDevice* context)
{
    m_Desc = desc;
    m_Buffer = buffer;
    m_Allocation = allocation;
    m_Allocator = allocator;
    m_Context = context;
}

VulkanBuffer::~VulkanBuffer()
{
    if (m_Buffer && m_Allocation)
    {
        if (m_MappedPtr)
        {
            vmaUnmapMemory(m_Allocator, m_Allocation);
        }
        m_Context->SafeReleaseResource(new VkBufferResource{ m_Buffer, m_Allocation, m_Allocator });
    }
}

void* VulkanBuffer::Map()
{
    if (m_Desc.Usage == ResourceUsage::Dynamic)
    {
        vk::Allocation allocation = m_Context->GetRingBuffer()->Allocate(m_Desc.Size, 16);

        m_Offset = allocation.Offset;

        return allocation.Mapped;
    }

    if (m_Desc.Usage == ResourceUsage::Readback)
    {
        if (!m_MappedPtr)
        {
            vmaMapMemory(m_Allocator, m_Allocation, &m_MappedPtr);
        }
        return m_MappedPtr;
    }

    return nullptr;
}

void VulkanBuffer::Unmap()
{
    if (m_Desc.Usage == ResourceUsage::Readback)
    {
        vmaUnmapMemory(m_Allocator, m_Allocation);
    }   
}

ResourceState VulkanBuffer::GetCurrentResourceState()
{
    return m_CurrentState;
}

}