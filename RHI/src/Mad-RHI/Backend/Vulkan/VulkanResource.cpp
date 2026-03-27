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

VulkanBuffer::VulkanBuffer()
{

}

VulkanBuffer::~VulkanBuffer()
{

}

ResourceState VulkanBuffer::GetCurrentResourceState()
{
    return m_CurrentState;
}

}