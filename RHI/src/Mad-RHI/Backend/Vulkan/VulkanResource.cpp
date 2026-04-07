#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"

namespace mad::rhi {

VulkanTexture::VulkanTexture(const TextureDesc& desc, VkImage image)
{
    m_Desc = desc;
    m_Image = image;
}

VulkanTexture::~VulkanTexture()
{
    
}

ResourceState VulkanTexture::GetCurrentResourceState()
{
    return m_CurrentState;
}

VulkanBuffer::VulkanBuffer(const BufferDesc& desc, VkBuffer buffer, VmaAllocation allocation, VmaAllocator allocator)
{
    m_Desc = desc;
    m_Buffer = buffer;
    m_Allocation = allocation;
    m_Allocator = allocator;
}

VulkanBuffer::~VulkanBuffer()
{
    if (m_Buffer && m_Allocation)
    {
        vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
    }
}

ResourceState VulkanBuffer::GetCurrentResourceState()
{
    return m_CurrentState;
}

}