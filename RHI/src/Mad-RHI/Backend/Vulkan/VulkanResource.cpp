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
        m_Context->SafeReleaseResource(new vk::VkImageResource{ m_Image, m_Allocation, m_Allocator }, m_Desc.QueueTypeFlags);
    }
}

ResourceState VulkanTexture::GetCurrentResourceState()
{
    return m_CurrentState;
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
        m_Context->SafeReleaseResource(new vk::VkBufferResource{ m_Buffer, m_Allocation, m_Allocator }, m_Desc.QueueTypeFlags);
    }
}

ResourceState VulkanBuffer::GetCurrentResourceState()
{
    return m_CurrentState;
}

}