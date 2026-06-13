#pragma once

#include "Mad-RHI/Resource.h"
#include <volk.h>
#include <vk_mem_alloc.h>
#include "Mad-RHI/Backend/Vulkan/Vk/ReleaseManager.h"
#include "Mad-RHI/Backend/Vulkan/Vk/RingAllocator.h"

namespace mad::rhi {

class VulkanDevice;
class VulkanTextureView;

class VulkanTexture : public ObjectBase<Texture>
{
protected:
    ~VulkanTexture();

public:
    VulkanTexture(const TextureDesc& desc, VkImage image, VulkanDevice* context);
    VulkanTexture(const TextureDesc& desc, VulkanDevice* context);

    virtual RefPtr<TextureView> GetDefaultSRV() override;
    virtual RefPtr<TextureView> GetDefaultRTV() override;
    virtual RefPtr<TextureView> GetDefaultDSV() override;

    virtual const TextureDesc& GetDesc() override;
    virtual ResourceState GetCurrentResourceState() override;

    VkImage GetImage() { return m_Image; }
    void SetResourceState(ResourceState state) { m_CurrentState = state; }

private:
    TextureDesc m_Desc;
    ResourceState m_CurrentState = ResourceState::Undefined;

    VkImage m_Image = nullptr;
    VmaAllocation m_Allocation = nullptr;

    VulkanDevice* m_Context = nullptr;

    VulkanTextureView* m_DefaultSRV = nullptr;
    VulkanTextureView* m_DefaultRTV = nullptr;
    VulkanTextureView* m_DefaultDSV = nullptr;
    
};

class VulkanBuffer : public ObjectBase<Buffer>
{
protected:
    ~VulkanBuffer();

public:
    VulkanBuffer(const BufferDesc& desc, VulkanDevice* context);

    virtual void* Map() override;
    virtual void Unmap() override;
        
    virtual const BufferDesc& GetDesc() override;
    virtual ResourceState GetCurrentResourceState() override;

    uint64_t GetID() { return m_ID; }
    VkBuffer GetBuffer() { return m_Buffer; }
    VmaAllocation GetAllocation() { return m_Allocation; }
    VkDeviceSize GetOffset() { return m_Offset; }
    
    void SetResourceState(ResourceState state) { m_CurrentState = state; }

private:
    ResourceState m_CurrentState = ResourceState::Undefined;
    BufferDesc m_Desc;

    uint64_t m_ID = 0;
    VkBuffer m_Buffer = nullptr;
    void* m_MappedPtr = nullptr;
    VmaAllocation m_Allocation = nullptr;

    VkDeviceSize m_Offset = 0;

    VulkanDevice* m_Context = nullptr;

};

class VulkanSampler : public ObjectBase<Sampler>
{
protected:
    ~VulkanSampler();

public:
    VulkanSampler(const SamplerDesc& desc, VulkanDevice* context);

    virtual const SamplerDesc& GetDesc() override;

    uint64_t GetID() { return m_ID; }
    VkSampler GetSampler() { return m_Sampler; }

private:
    SamplerDesc m_Desc;

    uint64_t m_ID = 0;
    VkSampler m_Sampler = nullptr;

    VulkanDevice* m_Context = nullptr;
    
};

class VulkanTextureView : public ObjectBase<TextureView>
{
friend class VulkanTexture;

protected:
    ~VulkanTextureView();

public:
    VulkanTextureView(RefCounter* sharedCounter, const TextureViewDesc& desc, 
        VulkanTexture* tex, VulkanDevice* context);

    uint64_t GetID() { return m_ID; }
    VulkanTexture* GetTexture() { return m_Texture; }
    VkImageView GetView() { return m_View; }

private:
    TextureViewDesc m_Desc;

    VkImageView m_View = nullptr;

    uint64_t m_ID = 0;
    VulkanDevice* m_Context = nullptr;
    VulkanTexture* m_Texture = nullptr;

    RefPtr<VulkanTexture> m_TextureRef = nullptr;

};

struct VkBufferResource : vk::StaleResourceBase
{
    VkBuffer Buffer;
    VmaAllocation Allocation;
    VmaAllocator Allocator;

    VkBufferResource(VkBuffer b, VmaAllocation a, VmaAllocator al)
        : Buffer(b), Allocation(a), Allocator(al) {}

    void Destroy() override
    {
        vmaDestroyBuffer(Allocator, Buffer, Allocation);
    }
};

struct VkImageResource : vk::StaleResourceBase 
{
    VkImage Image;
    VmaAllocation Allocation;
    VmaAllocator Allocator;
    
    VkImageResource(VkImage i, VmaAllocation a, VmaAllocator al)
        : Image(i), Allocation(a), Allocator(al) {}

    void Destroy() override { vmaDestroyImage(Allocator, Image, Allocation); }
};

struct VkSamplerResource : vk::StaleResourceBase
{
    VkSampler Sampler;
    VkDevice Device;

    VkSamplerResource(VkSampler s, VkDevice d) : Sampler(s), Device(d) {}

    void Destroy() override { vkDestroySampler(Device, Sampler, nullptr); }
};

struct VkImageViewResource : vk::StaleResourceBase
{
    VkImageView View;
    VkDevice Device;

    VkImageViewResource(VkImageView v, VkDevice d) : View(v), Device(d) {}

    void Destroy() override { vkDestroyImageView(Device, View, nullptr); }
};

struct VkRingBufferResource : vk::StaleResourceBase 
{
    VkDeviceSize Head;
    vk::RingBuffer* BufferPtr;
    
    VkRingBufferResource(VkDeviceSize head, vk::RingBuffer* bufferPtr)
        : Head(head), BufferPtr(bufferPtr) {}

    void Destroy() override { BufferPtr->SetTail(Head); }
};

inline bool IsDepthOnlyFormat(TextureFormat fmt)
{
    return fmt == TextureFormat::D16_UNorm || fmt == TextureFormat::D32_SFloat;
}

inline bool IsDepthStencilFormat(TextureFormat fmt)
{
    return fmt == TextureFormat::D24_UNorm_S8_UInt || fmt == TextureFormat::D32_SFloat_S8_UInt;
}

inline VkImageViewType ToVkImageViewType(TextureDimension dimension)
{
    switch (dimension)
    {
    case TextureDimension::Texture1D:           return VK_IMAGE_VIEW_TYPE_1D;
    case TextureDimension::Texture1DArray:      return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case TextureDimension::Texture2D:           return VK_IMAGE_VIEW_TYPE_2D;
    case TextureDimension::Texture2DArray:      return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case TextureDimension::Texture3D:           return VK_IMAGE_VIEW_TYPE_3D;
    case TextureDimension::TextureCube:         return VK_IMAGE_VIEW_TYPE_CUBE;
    case TextureDimension::TextureCubeArray:    return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

    default:                                    return VK_IMAGE_VIEW_TYPE_2D;
    }
}

inline VkFormat ToVkFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::Unknown:                        return VK_FORMAT_UNDEFINED;

    case TextureFormat::R8_UNorm:                       return VK_FORMAT_R8_UNORM;
    case TextureFormat::R8_SNorm:                       return VK_FORMAT_R8_SNORM;
    case TextureFormat::R8_UInt:                        return VK_FORMAT_R8_UINT;
    case TextureFormat::R8_SInt:                        return VK_FORMAT_R8_SINT;

    case TextureFormat::R16_UNorm:                      return VK_FORMAT_R16_UNORM;
    case TextureFormat::R16_SNorm:                      return VK_FORMAT_R16_SNORM;
    case TextureFormat::R16_UInt:                       return VK_FORMAT_R16_UINT;
    case TextureFormat::R16_SInt:                       return VK_FORMAT_R16_SINT;
    case TextureFormat::R16_SFloat:                     return VK_FORMAT_R16_SFLOAT;
    case TextureFormat::R8G8_UNorm:                     return VK_FORMAT_R8G8_UNORM;
    case TextureFormat::R8G8_SNorm:                     return VK_FORMAT_R8G8_SNORM;
    case TextureFormat::R8G8_UInt:                      return VK_FORMAT_R8G8_UINT;
    case TextureFormat::R8G8_SInt:                      return VK_FORMAT_R8G8_SINT;

    case TextureFormat::R32_UInt:                       return VK_FORMAT_R32_UINT;
    case TextureFormat::R32_SInt:                       return VK_FORMAT_R32_SINT;
    case TextureFormat::R32_SFloat:                     return VK_FORMAT_R32_SFLOAT;
    case TextureFormat::R16G16_UNorm:                   return VK_FORMAT_R16G16_UNORM;
    case TextureFormat::R16G16_SNorm:                   return VK_FORMAT_R16G16_SNORM;
    case TextureFormat::R16G16_UInt:                    return VK_FORMAT_R16G16_UINT;
    case TextureFormat::R16G16_SInt:                    return VK_FORMAT_R16G16_SINT;
    case TextureFormat::R16G16_SFloat:                  return VK_FORMAT_R16G16_SFLOAT;

    case TextureFormat::R8G8B8A8_UNorm:                 return VK_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::R8G8B8A8_SRGB_UNorm:            return VK_FORMAT_R8G8B8A8_SRGB;
    case TextureFormat::R8G8B8A8_SNorm:                 return VK_FORMAT_R8G8B8A8_SNORM;
    case TextureFormat::R8G8B8A8_UInt:                  return VK_FORMAT_R8G8B8A8_UINT;
    case TextureFormat::R8G8B8A8_SInt:                  return VK_FORMAT_R8G8B8A8_SINT;
    case TextureFormat::B8G8R8A8_UNorm:                 return VK_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::B8G8R8A8_SRGB_UNorm:            return VK_FORMAT_B8G8R8A8_SRGB;

    case TextureFormat::R32G32_UInt:                    return VK_FORMAT_R32G32_UINT;
    case TextureFormat::R32G32_SInt:                    return VK_FORMAT_R32G32_SINT;
    case TextureFormat::R32G32_SFloat:                  return VK_FORMAT_R32G32_SFLOAT;
    case TextureFormat::R16G16B16A16_UNorm:             return VK_FORMAT_R16G16B16A16_UNORM;
    case TextureFormat::R16G16B16A16_SNorm:             return VK_FORMAT_R16G16B16A16_SNORM;
    case TextureFormat::R16G16B16A16_UInt:              return VK_FORMAT_R16G16B16A16_UINT;
    case TextureFormat::R16G16B16A16_SInt:              return VK_FORMAT_R16G16B16A16_SINT;
    case TextureFormat::R16G16B16A16_SFloat:            return VK_FORMAT_R16G16B16A16_SFLOAT;

    case TextureFormat::R32G32B32_UInt:                 return VK_FORMAT_R32G32B32_UINT;
    case TextureFormat::R32G32B32_SInt:                 return VK_FORMAT_R32G32B32_SINT;
    case TextureFormat::R32G32B32_SFloat:               return VK_FORMAT_R32G32B32_SFLOAT;
    case TextureFormat::R32G32B32A32_UInt:              return VK_FORMAT_R32G32B32A32_UINT;
    case TextureFormat::R32G32B32A32_SInt:              return VK_FORMAT_R32G32B32A32_SINT;
    case TextureFormat::R32G32B32A32_SFloat:            return VK_FORMAT_R32G32B32A32_SFLOAT;

    case TextureFormat::D16_UNorm:                      return VK_FORMAT_D16_UNORM;
    case TextureFormat::D32_SFloat:                     return VK_FORMAT_D32_SFLOAT;
    case TextureFormat::D24_UNorm_S8_UInt:              return VK_FORMAT_D24_UNORM_S8_UINT;
    case TextureFormat::D32_SFloat_S8_UInt:             return VK_FORMAT_D32_SFLOAT_S8_UINT;

    case TextureFormat::BC1_RGBA_UNorm_Block:           return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case TextureFormat::BC1_RGBA_SRGB_UNorm_Block:      return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case TextureFormat::BC2_UNorm_Block:                return VK_FORMAT_BC2_UNORM_BLOCK;
    case TextureFormat::BC2_SRGB_UNorm_Block:           return VK_FORMAT_BC2_SRGB_BLOCK;
    case TextureFormat::BC3_UNorm_Block:                return VK_FORMAT_BC3_UNORM_BLOCK;
    case TextureFormat::BC3_SRGB_UNorm_Block:           return VK_FORMAT_BC3_SRGB_BLOCK;
    case TextureFormat::BC4_UNorm_Block:                return VK_FORMAT_BC4_UNORM_BLOCK;
    case TextureFormat::BC4_SNorm_Block:                return VK_FORMAT_BC4_SNORM_BLOCK;
    case TextureFormat::BC5_UNorm_Block:                return VK_FORMAT_BC5_UNORM_BLOCK;
    case TextureFormat::BC5_SNorm_Block:                return VK_FORMAT_BC5_SNORM_BLOCK;
    case TextureFormat::BC6H_UFloat_Block:              return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case TextureFormat::BC6H_SFloat_Block:              return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case TextureFormat::BC7_UNorm_Block:                return VK_FORMAT_BC7_UNORM_BLOCK;
    case TextureFormat::BC7_SRGB_UNorm_Block:           return VK_FORMAT_BC7_SRGB_BLOCK;

    default:                                            return VK_FORMAT_UNDEFINED;
    }
}

inline TextureDimension FromVkImageViewType(VkImageViewType type)
{
    switch (type)
    {
    case VK_IMAGE_VIEW_TYPE_1D:         return TextureDimension::Texture1D;
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:   return TextureDimension::Texture1DArray;
    case VK_IMAGE_VIEW_TYPE_2D:         return TextureDimension::Texture2D;
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:   return TextureDimension::Texture2DArray;
    case VK_IMAGE_VIEW_TYPE_3D:         return TextureDimension::Texture3D;
    case VK_IMAGE_VIEW_TYPE_CUBE:       return TextureDimension::TextureCube;
    case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY: return TextureDimension::TextureCubeArray;

    default:                            return TextureDimension::Texture2D;
    }
}

// TODO: remove unnesessary
inline TextureFormat FromVkFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_UNDEFINED:                       return TextureFormat::Unknown;

    case VK_FORMAT_R8_UNORM:                        return TextureFormat::R8_UNorm;
    case VK_FORMAT_R8_SNORM:                        return TextureFormat::R8_SNorm;
    case VK_FORMAT_R8_UINT:                         return TextureFormat::R8_UInt;
    case VK_FORMAT_R8_SINT:                         return TextureFormat::R8_SInt;

    case VK_FORMAT_R16_UNORM:                       return TextureFormat::R16_UNorm;
    case VK_FORMAT_R16_SNORM:                       return TextureFormat::R16_SNorm;
    case VK_FORMAT_R16_UINT:                        return TextureFormat::R16_UInt;
    case VK_FORMAT_R16_SINT:                        return TextureFormat::R16_SInt;
    case VK_FORMAT_R16_SFLOAT:                      return TextureFormat::R16_SFloat;
    case VK_FORMAT_R8G8_UNORM:                      return TextureFormat::R8G8_UNorm;
    case VK_FORMAT_R8G8_SNORM:                      return TextureFormat::R8G8_SNorm;
    case VK_FORMAT_R8G8_UINT:                       return TextureFormat::R8G8_UInt;
    case VK_FORMAT_R8G8_SINT:                       return TextureFormat::R8G8_SInt;

    case VK_FORMAT_R32_UINT:                        return TextureFormat::R32_UInt;
    case VK_FORMAT_R32_SINT:                        return TextureFormat::R32_SInt;
    case VK_FORMAT_R32_SFLOAT:                      return TextureFormat::R32_SFloat;
    case VK_FORMAT_R16G16_UNORM:                    return TextureFormat::R16G16_UNorm;
    case VK_FORMAT_R16G16_SNORM:                    return TextureFormat::R16G16_SNorm;
    case VK_FORMAT_R16G16_UINT:                     return TextureFormat::R16G16_UInt;
    case VK_FORMAT_R16G16_SINT:                     return TextureFormat::R16G16_SInt;
    case VK_FORMAT_R16G16_SFLOAT:                   return TextureFormat::R16G16_SFloat;

    case VK_FORMAT_R8G8B8A8_UNORM:                  return TextureFormat::R8G8B8A8_UNorm;
    case VK_FORMAT_R8G8B8A8_SRGB:                   return TextureFormat::R8G8B8A8_SRGB_UNorm;
    case VK_FORMAT_R8G8B8A8_SNORM:                  return TextureFormat::R8G8B8A8_SNorm;
    case VK_FORMAT_R8G8B8A8_UINT:                   return TextureFormat::R8G8B8A8_UInt;
    case VK_FORMAT_R8G8B8A8_SINT:                   return TextureFormat::R8G8B8A8_SInt;
    case VK_FORMAT_B8G8R8A8_UNORM:                  return TextureFormat::B8G8R8A8_UNorm;
    case VK_FORMAT_B8G8R8A8_SRGB:                   return TextureFormat::B8G8R8A8_SRGB_UNorm;

    case VK_FORMAT_R32G32_UINT:                     return TextureFormat::R32G32_UInt;
    case VK_FORMAT_R32G32_SINT:                     return TextureFormat::R32G32_SInt;
    case VK_FORMAT_R32G32_SFLOAT:                   return TextureFormat::R32G32_SFloat;
    case VK_FORMAT_R16G16B16A16_UNORM:              return TextureFormat::R16G16B16A16_UNorm;
    case VK_FORMAT_R16G16B16A16_SNORM:              return TextureFormat::R16G16B16A16_SNorm;
    case VK_FORMAT_R16G16B16A16_UINT:               return TextureFormat::R16G16B16A16_UInt;
    case VK_FORMAT_R16G16B16A16_SINT:               return TextureFormat::R16G16B16A16_SInt;
    case VK_FORMAT_R16G16B16A16_SFLOAT:             return TextureFormat::R16G16B16A16_SFloat;

    case VK_FORMAT_R32G32B32_UINT:                  return TextureFormat::R32G32B32_UInt;
    case VK_FORMAT_R32G32B32_SINT:                  return TextureFormat::R32G32B32_SInt;
    case VK_FORMAT_R32G32B32_SFLOAT:                return TextureFormat::R32G32B32_SFloat;
    case VK_FORMAT_R32G32B32A32_UINT:               return TextureFormat::R32G32B32A32_UInt;
    case VK_FORMAT_R32G32B32A32_SINT:               return TextureFormat::R32G32B32A32_SInt;
    case VK_FORMAT_R32G32B32A32_SFLOAT:             return TextureFormat::R32G32B32A32_SFloat;

    case VK_FORMAT_D16_UNORM:                       return TextureFormat::D16_UNorm;
    case VK_FORMAT_D32_SFLOAT:                      return TextureFormat::D32_SFloat;
    case VK_FORMAT_D24_UNORM_S8_UINT:               return TextureFormat::D24_UNorm_S8_UInt;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:              return TextureFormat::D32_SFloat_S8_UInt;

    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:            return TextureFormat::BC1_RGBA_UNorm_Block;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:             return TextureFormat::BC1_RGBA_SRGB_UNorm_Block;
    case VK_FORMAT_BC2_UNORM_BLOCK:                 return TextureFormat::BC2_UNorm_Block;
    case VK_FORMAT_BC2_SRGB_BLOCK:                  return TextureFormat::BC2_SRGB_UNorm_Block;
    case VK_FORMAT_BC3_UNORM_BLOCK:                 return TextureFormat::BC3_UNorm_Block;
    case VK_FORMAT_BC3_SRGB_BLOCK:                  return TextureFormat::BC3_SRGB_UNorm_Block;
    case VK_FORMAT_BC4_UNORM_BLOCK:                 return TextureFormat::BC4_UNorm_Block;
    case VK_FORMAT_BC4_SNORM_BLOCK:                 return TextureFormat::BC4_SNorm_Block;
    case VK_FORMAT_BC5_UNORM_BLOCK:                 return TextureFormat::BC5_UNorm_Block;
    case VK_FORMAT_BC5_SNORM_BLOCK:                 return TextureFormat::BC5_SNorm_Block;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:               return TextureFormat::BC6H_UFloat_Block;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:               return TextureFormat::BC6H_SFloat_Block;
    case VK_FORMAT_BC7_UNORM_BLOCK:                 return TextureFormat::BC7_UNorm_Block;
    case VK_FORMAT_BC7_SRGB_BLOCK:                  return TextureFormat::BC7_SRGB_UNorm_Block;

    default:                                        return TextureFormat::Unknown;
    }
}

inline VkImageLayout ToVkImageLayout(ResourceState state)
{
    switch (state)
    {
    case ResourceState::Undefined:        return VK_IMAGE_LAYOUT_UNDEFINED;
    case ResourceState::RenderTarget:     return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ResourceState::ShaderResource:   return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ResourceState::UnorderedAccess:  return VK_IMAGE_LAYOUT_GENERAL;
    case ResourceState::DepthWrite:       return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ResourceState::DepthRead:        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ResourceState::CopyDst:          return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ResourceState::CopySrc:          return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ResourceState::Present:          return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default:                              return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

inline ResourceState FromVkImageLayout(VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:                         return ResourceState::Undefined;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:          return ResourceState::RenderTarget;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:          return ResourceState::ShaderResource;
    case VK_IMAGE_LAYOUT_GENERAL:                           return ResourceState::UnorderedAccess;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:  return ResourceState::DepthWrite;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:   return ResourceState::DepthRead;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:              return ResourceState::CopyDst;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:              return ResourceState::CopySrc;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:                   return ResourceState::Present;

    default:                                                return ResourceState::Undefined;
    }
}

inline VkAccessFlags ToVkAccessMask(ResourceState state)
{
    switch (state)
    {
    case ResourceState::Undefined:        return 0;
    case ResourceState::VertexBuffer:     return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    case ResourceState::IndexBuffer:      return VK_ACCESS_INDEX_READ_BIT;
    case ResourceState::RenderTarget:     return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case ResourceState::ShaderResource:   return VK_ACCESS_SHADER_READ_BIT;
    case ResourceState::UnorderedAccess:  return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case ResourceState::DepthWrite:       return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case ResourceState::DepthRead:        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case ResourceState::CopyDst:          return VK_ACCESS_TRANSFER_WRITE_BIT;
    case ResourceState::CopySrc:          return VK_ACCESS_TRANSFER_READ_BIT;
    case ResourceState::Present:          return 0;

    default:                              return 0;
    }
}

inline VkFilter ToVkFilter(FilterType filter)
{
    switch (filter)
    {
    case FilterType::Nearest:   return VK_FILTER_NEAREST;
    case FilterType::Linear:    return VK_FILTER_LINEAR;
    default:                    return VK_FILTER_LINEAR;
    }
}

inline VkSamplerMipmapMode ToVkMipmapMode(FilterType mipFilter)
{
    switch (mipFilter)
    {
    case FilterType::Nearest:    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case FilterType::Linear:     return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default:                     return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

inline VkBorderColor ToVkBorderColor(BorderColor border)
{
    switch (border)
    {
    case BorderColor::FloatTransparentBlack:    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case BorderColor::IntTransparentBlack:      return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    case BorderColor::FloatOpaqueBlack:         return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case BorderColor::IntOpaqueBlack:           return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    case BorderColor::FloatOpaqueWhite:         return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    case BorderColor::IntOpaqueWhite:           return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    default:                                    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    }
}

inline VkSamplerAddressMode ToVkAddressMode(TextureAddressMode mode)
{
    switch (mode)
    {
    case TextureAddressMode::Repeat:            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TextureAddressMode::MirroredRepeat:    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TextureAddressMode::ClampToEdge:       return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureAddressMode::ClampToBorder:     return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case TextureAddressMode::MirrorClampToEdge: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:                                    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
}

inline VkCompareOp ToVkCompareOp(CompareOp op)
{
    switch (op)
    {
    case CompareOp::Never:        return VK_COMPARE_OP_NEVER;
    case CompareOp::Less:         return VK_COMPARE_OP_LESS;
    case CompareOp::Equal:        return VK_COMPARE_OP_EQUAL;
    case CompareOp::LessEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::Greater:      return VK_COMPARE_OP_GREATER;
    case CompareOp::NotEqual:     return VK_COMPARE_OP_NOT_EQUAL;
    case CompareOp::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case CompareOp::Always:       return VK_COMPARE_OP_ALWAYS;
    default:                      return VK_COMPARE_OP_NEVER;
    }
}

}