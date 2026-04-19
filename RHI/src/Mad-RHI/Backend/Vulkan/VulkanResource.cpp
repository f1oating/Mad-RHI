#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

namespace mad::rhi {

VulkanTexture::VulkanTexture(const TextureDesc& desc, VkImage image, VulkanDevice* context)
{
    m_Desc = desc;
    m_Image = image;
    m_Context = context;
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
    if (desc.BindFlags & RESOURCE_BIND_RENDER_TARGET) usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (desc.BindFlags & RESOURCE_BIND_DEPTH_STENCIL) usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (desc.BindFlags & RESOURCE_BIND_SHADER_RESOURSE) usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (desc.BindFlags & RESOURCE_BIND_UNORDERED_ACCESS) usage |= VK_IMAGE_USAGE_STORAGE_BIT;

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
    if (m_DefaultSRV)
    {
        delete m_DefaultSRV;
        m_DefaultSRV = nullptr;
    }

    if (m_DefaultRTV)
    {
        delete m_DefaultRTV;
        m_DefaultRTV = nullptr;
    }

    if (m_Image && m_Allocation)
    {
        m_Context->SafeReleaseResource(new VkImageResource{ m_Image, m_Allocation, m_Context->GetVmaAllocator() });
    }
}

RefPtr<TextureView> VulkanTexture::GetDefaultSRV()
{
    if (!m_DefaultSRV)
    {
        TextureViewDesc desc;
        desc.ViewType = TextureViewType::SRV;
        desc.Format = m_Desc.Format;
        desc.MostDetailedMip = 0;
        desc.NumMipLevels = 0;
        desc.FirstArraySlice = 0;
        desc.NumArraySlices = 0;

        m_DefaultSRV = new VulkanTextureView(GetRefCounter(), desc, this, m_Context);
    }

    return RefPtr<TextureView>(m_DefaultSRV);
}

RefPtr<TextureView> VulkanTexture::GetDefaultRTV()
{
    if (!m_DefaultRTV)
    {
        TextureViewDesc desc;
        desc.ViewType = TextureViewType::RTV;
        desc.Format = m_Desc.Format;
        desc.MostDetailedMip = 0;
        desc.NumMipLevels = 1;
        desc.FirstArraySlice = 0;
        desc.NumArraySlices = 0;

        m_DefaultRTV = new VulkanTextureView(GetRefCounter(), desc, this, m_Context);
    }

    return RefPtr<TextureView>(m_DefaultRTV);
}

const TextureDesc& VulkanTexture::GetDesc()
{
    return m_Desc;
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
    if (desc.BindFlags & RESOURCE_BIND_VERTEX_BUFFER) usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (desc.BindFlags & RESOURCE_BIND_INDEX_BUFFER) usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (desc.BindFlags & RESOURCE_BIND_UNIFORM_BUFFER) usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (desc.BindFlags & RESOURCE_BIND_SHADER_RESOURSE) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (desc.BindFlags & RESOURCE_BIND_UNORDERED_ACCESS) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

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

const BufferDesc& VulkanBuffer::GetDesc()
{
    return m_Desc;
}

ResourceState VulkanBuffer::GetCurrentResourceState()
{
    return m_CurrentState;
}

VulkanSampler::VulkanSampler(const SamplerDesc& desc, VulkanDevice* context)
{
    m_Desc = desc;
    m_Context = context;

    VkSamplerCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.magFilter = ToVkFilter(desc.MagFilter);
    sci.minFilter = ToVkFilter(desc.MinFilter);
    sci.mipmapMode = ToVkMipmapMode(desc.MipFilter);
    sci.addressModeU = ToVkAddressMode(desc.AddressU);
    sci.addressModeV = ToVkAddressMode(desc.AddressV);
    sci.addressModeW = ToVkAddressMode(desc.AddressW);
    sci.mipLodBias = desc.MipLodBias;
    sci.anisotropyEnable = (desc.MaxAnisotropy > 1) ? VK_TRUE : VK_FALSE;
    sci.maxAnisotropy = static_cast<float>(desc.MaxAnisotropy);
    sci.compareEnable = IsComparisonFilter(desc.MinFilter) ? VK_TRUE : VK_FALSE;
    sci.compareOp = ToVkCompareOp(desc.Compare);
    sci.minLod = desc.MinLod;
    sci.maxLod = desc.MaxLod;
    sci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    sci.unnormalizedCoordinates = VK_FALSE;

    vkCreateSampler(m_Context->GetDevice(), &sci, nullptr, &m_Sampler);
}

VulkanSampler::~VulkanSampler()
{
    if (m_Sampler)
    {
        m_Context->SafeReleaseResource(new VkSamplerResource{ m_Sampler, m_Context->GetDevice() });
    }
}

const SamplerDesc& VulkanSampler::GetDesc()
{
    return m_Desc;
}

VulkanTextureView::VulkanTextureView(RefCounter* sharedCounter, const TextureViewDesc& desc, 
    VulkanTexture* tex, VulkanDevice* context)
    : ObjectBase<TextureView>(sharedCounter)
{
    m_Desc = desc;
    m_Context = context;
    m_Texture = tex;

    if (m_Desc.Format == TextureFormat::Unknown)
        m_Desc.Format = tex->GetDesc().Format;

    VkImageViewCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ci.image = tex->GetImage();

    switch (tex->GetDesc().Dimension) 
    {
        case TextureDimension::Texture1D:           ci.viewType = VK_IMAGE_VIEW_TYPE_1D; break;
        case TextureDimension::Texture1DArray:      ci.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
        case TextureDimension::Texture2D:           ci.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
        case TextureDimension::Texture2DArray:      ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
        case TextureDimension::TextureCube:         ci.viewType = VK_IMAGE_VIEW_TYPE_CUBE; break;
        case TextureDimension::TextureCubeArray:    ci.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; break;
        case TextureDimension::Texture3D:           ci.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
    }

    ci.format = ToVkFormat(m_Desc.Format);
    ci.components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY
    };
    ci.subresourceRange.baseMipLevel = m_Desc.MostDetailedMip;
    ci.subresourceRange.levelCount = (m_Desc.NumMipLevels == 0)
        ? VK_REMAINING_MIP_LEVELS
        : m_Desc.NumMipLevels;
    ci.subresourceRange.baseArrayLayer = m_Desc.FirstArraySlice;
    ci.subresourceRange.layerCount = (m_Desc.NumArraySlices == 0)
        ? VK_REMAINING_ARRAY_LAYERS
        : m_Desc.NumArraySlices;

    if (m_Desc.ViewType == TextureViewType::DSV)
    {
        if (IsDepthOnlyFormat(m_Desc.Format))
            ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        else
            ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else if (m_Desc.ViewType == TextureViewType::SRV && IsDepthStencilFormat(m_Desc.Format))
    {
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
    {
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    vkCreateImageView(m_Context->GetDevice(), &ci, nullptr, &m_View);
}

VulkanTextureView::~VulkanTextureView()
{
    if (m_View)
    {
        m_Context->SafeReleaseResource(new VkImageViewResource{ m_View, m_Context->GetDevice() });
    }   
}

}