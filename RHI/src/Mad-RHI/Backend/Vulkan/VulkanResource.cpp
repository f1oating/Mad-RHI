#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

namespace mad::rhi {

VulkanTexture::VulkanTexture(const TextureDesc& desc, VkImage image)
{
    m_Desc = desc;
    m_Image = image;
}

VulkanTexture::VulkanTexture(const TextureDesc& desc, VulkanDevice* context)
{
    m_Desc = desc;
    m_Context = context;

    VkImageType imageType;
    switch (desc.Dimension)
    {
    case TextureDimension::Texture1D:
    case TextureDimension::Texture1DArray:
        imageType = VK_IMAGE_TYPE_1D; break;
    case TextureDimension::Texture3D:
        imageType = VK_IMAGE_TYPE_3D; break;
    default:
        imageType = VK_IMAGE_TYPE_2D; break;
    }

    bool isCube = (desc.Dimension == TextureDimension::TextureCube ||
        desc.Dimension == TextureDimension::TextureCubeArray);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (desc.BindFlags & RenderTarget) usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (desc.BindFlags & DepthStencil) usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (desc.BindFlags & ShaderResource) usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (desc.BindFlags & UnorderedAccess) usage |= VK_IMAGE_USAGE_STORAGE_BIT;

    VkImageCreateInfo ici{};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = imageType;
    ici.format = ToVkFormat(desc.Format);
    ici.extent = { desc.Width, desc.Height, (imageType == VK_IMAGE_TYPE_3D) ? desc.Depth : 1 };
    ici.mipLevels = desc.MipLevels;
    ici.arrayLayers = isCube ? desc.ArraySize * 6 : desc.ArraySize;
    ici.samples = static_cast<VkSampleCountFlagBits>(desc.SampleCount);
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = usage;
    ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (isCube) ici.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    vmaCreateImage(m_Context->GetVmaAllocator(), &ici, &aci, &m_Image, &m_Allocation, nullptr);
}

VulkanTexture::~VulkanTexture()
{
    if (m_Image && m_Allocation)
    {
        m_Context->SafeReleaseResource(new VkImageResource{ m_Image, m_Allocation, m_Context->GetVmaAllocator() });
    }
}

ResourceState VulkanTexture::GetCurrentResourceState()
{
    return m_CurrentState;
}

VulkanBuffer::VulkanBuffer(const BufferDesc& desc, VulkanDevice* context)
{
    m_Desc = desc;
    m_Context = context;

    if (desc.Usage == ResourceUsage::Dynamic)
    {
        vk::RingBuffer* rb = m_Context->GetRingBuffer();
        m_Buffer = rb->GetBuffer();
        m_MappedPtr = rb->GetMappedPtr();
        
        return;
    }

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (desc.BindFlags & VertexBuffer) usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (desc.BindFlags & IndexBuffer) usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (desc.BindFlags & UniformBuffer) usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (desc.BindFlags & ShaderResource) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (desc.BindFlags & UnorderedAccess) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBufferCreateInfo bci{};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = desc.Size;
    bci.usage = usage;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo aci{};
    if (desc.Usage == ResourceUsage::Readback)
    {
        aci.usage = VMA_MEMORY_USAGE_AUTO;
        aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        bci.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    else
    {
        aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    vmaCreateBuffer(m_Context->GetVmaAllocator(), &bci, &aci, &m_Buffer, &m_Allocation, nullptr);
}

VulkanBuffer::~VulkanBuffer()
{
    if (m_Buffer && m_Allocation)
    {
        if (m_MappedPtr)
        {
            vmaUnmapMemory(m_Context->GetVmaAllocator(), m_Allocation);
        }
        m_Context->SafeReleaseResource(new VkBufferResource{ m_Buffer, m_Allocation, m_Context->GetVmaAllocator() });
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
            vmaMapMemory(m_Context->GetVmaAllocator(), m_Allocation, &m_MappedPtr);
        }
        return m_MappedPtr;
    }

    return nullptr;
}

void VulkanBuffer::Unmap()
{
    if (m_Desc.Usage == ResourceUsage::Readback)
    {
        vmaUnmapMemory(m_Context->GetVmaAllocator(), m_Allocation);
    }   
}

ResourceState VulkanBuffer::GetCurrentResourceState()
{
    return m_CurrentState;
}

}