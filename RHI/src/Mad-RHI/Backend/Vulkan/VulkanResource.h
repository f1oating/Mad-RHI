#pragma once

#include "Mad-RHI/Resource.h"
#include <volk/volk.h>
#include <vk_mem_alloc.h>

namespace mad::rhi {

class VulkanTexture : public ObjectBase<Texture>
{
protected:
    ~VulkanTexture();

public:
    VulkanTexture(const TextureDesc& desc, VkImage image);

    virtual ResourceState GetCurrentResourceState() override;

    VkImage GetImage() { return m_Image; }
    void SetResourceState(ResourceState state) { m_CurrentState = state; }

private:
    TextureDesc m_Desc;
    ResourceState m_CurrentState = ResourceState::Undefined;

    VkImage m_Image = nullptr;

};

class VulkanBuffer : public ObjectBase<Buffer>
{
protected:
    ~VulkanBuffer();

public:
    VulkanBuffer(const BufferDesc& desc, VkBuffer buffer, VmaAllocation allocation, VmaAllocator allocator);

    virtual ResourceState GetCurrentResourceState() override;

    VkBuffer GetBuffer() { return m_Buffer; }
    VmaAllocation GetAllocation() { return m_Allocation; }
    
    void SetResourceState(ResourceState state) { m_CurrentState = state; }

private:
    ResourceState m_CurrentState = ResourceState::Undefined;
    BufferDesc m_Desc;

    VkBuffer m_Buffer = nullptr;
    VmaAllocation m_Allocation = nullptr;
    VmaAllocator m_Allocator = nullptr;

};

inline VkImageViewType ToVkImageViewType(TextureDimension dimension)
{
    switch (dimension)
    {
    case TextureDimension::Texture1D:           return VK_IMAGE_VIEW_TYPE_1D;
    case TextureDimension::Texture1DArray:      return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case TextureDimension::Texture2D:           return VK_IMAGE_VIEW_TYPE_2D;
    case TextureDimension::Texture2DArray:      return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case TextureDimension::Texture2DMS:         return VK_IMAGE_VIEW_TYPE_2D;
    case TextureDimension::Texture2DMSArray:    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
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
    case TextureFormat::Unknown:             return VK_FORMAT_UNDEFINED;

    case TextureFormat::R8_UNorm:            return VK_FORMAT_R8_UNORM;
    case TextureFormat::R8_SNorm:            return VK_FORMAT_R8_SNORM;
    case TextureFormat::R8_UInt:             return VK_FORMAT_R8_UINT;
    case TextureFormat::R8_SInt:             return VK_FORMAT_R8_SINT;

    case TextureFormat::R16_UNorm:           return VK_FORMAT_R16_UNORM;
    case TextureFormat::R16_SNorm:           return VK_FORMAT_R16_SNORM;
    case TextureFormat::R16_UInt:            return VK_FORMAT_R16_UINT;
    case TextureFormat::R16_SInt:            return VK_FORMAT_R16_SINT;
    case TextureFormat::R16_Float:           return VK_FORMAT_R16_SFLOAT;
    case TextureFormat::RG8_UNorm:           return VK_FORMAT_R8G8_UNORM;
    case TextureFormat::RG8_SNorm:           return VK_FORMAT_R8G8_SNORM;
    case TextureFormat::RG8_UInt:            return VK_FORMAT_R8G8_UINT;
    case TextureFormat::RG8_SInt:            return VK_FORMAT_R8G8_SINT;

    case TextureFormat::R32_UInt:            return VK_FORMAT_R32_UINT;
    case TextureFormat::R32_SInt:            return VK_FORMAT_R32_SINT;
    case TextureFormat::R32_Float:           return VK_FORMAT_R32_SFLOAT;
    case TextureFormat::RG16_UNorm:          return VK_FORMAT_R16G16_UNORM;
    case TextureFormat::RG16_SNorm:          return VK_FORMAT_R16G16_SNORM;
    case TextureFormat::RG16_UInt:           return VK_FORMAT_R16G16_UINT;
    case TextureFormat::RG16_SInt:           return VK_FORMAT_R16G16_SINT;
    case TextureFormat::RG16_Float:          return VK_FORMAT_R16G16_SFLOAT;

    case TextureFormat::RGBA8_UNorm:         return VK_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::RGBA8_UNorm_SRGB:    return VK_FORMAT_R8G8B8A8_SRGB;
    case TextureFormat::RGBA8_SNorm:         return VK_FORMAT_R8G8B8A8_SNORM;
    case TextureFormat::RGBA8_UInt:          return VK_FORMAT_R8G8B8A8_UINT;
    case TextureFormat::RGBA8_SInt:          return VK_FORMAT_R8G8B8A8_SINT;
    case TextureFormat::BGRA8_UNorm:         return VK_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::BGRA8_UNorm_SRGB:    return VK_FORMAT_B8G8R8A8_SRGB;
    case TextureFormat::RGB10A2_UNorm:       return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    case TextureFormat::RGB10A2_UInt:        return VK_FORMAT_A2B10G10R10_UINT_PACK32;
    case TextureFormat::RG11B10_Float:       return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

    case TextureFormat::RG32_UInt:           return VK_FORMAT_R32G32_UINT;
    case TextureFormat::RG32_SInt:           return VK_FORMAT_R32G32_SINT;
    case TextureFormat::RG32_Float:          return VK_FORMAT_R32G32_SFLOAT;
    case TextureFormat::RGBA16_UNorm:        return VK_FORMAT_R16G16B16A16_UNORM;
    case TextureFormat::RGBA16_SNorm:        return VK_FORMAT_R16G16B16A16_SNORM;
    case TextureFormat::RGBA16_UInt:         return VK_FORMAT_R16G16B16A16_UINT;
    case TextureFormat::RGBA16_SInt:         return VK_FORMAT_R16G16B16A16_SINT;
    case TextureFormat::RGBA16_Float:        return VK_FORMAT_R16G16B16A16_SFLOAT;

    case TextureFormat::RGB32_UInt:          return VK_FORMAT_R32G32B32_UINT;
    case TextureFormat::RGB32_SInt:          return VK_FORMAT_R32G32B32_SINT;
    case TextureFormat::RGB32_Float:         return VK_FORMAT_R32G32B32_SFLOAT;
    case TextureFormat::RGBA32_UInt:         return VK_FORMAT_R32G32B32A32_UINT;
    case TextureFormat::RGBA32_SInt:         return VK_FORMAT_R32G32B32A32_SINT;
    case TextureFormat::RGBA32_Float:        return VK_FORMAT_R32G32B32A32_SFLOAT;

    case TextureFormat::D16_UNorm:           return VK_FORMAT_D16_UNORM;
    case TextureFormat::D32_Float:           return VK_FORMAT_D32_SFLOAT;
    case TextureFormat::D24_UNorm_S8_UInt:   return VK_FORMAT_D24_UNORM_S8_UINT;
    case TextureFormat::D32_Float_S8_UInt:   return VK_FORMAT_D32_SFLOAT_S8_UINT;

    case TextureFormat::BC1_UNorm:           return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case TextureFormat::BC1_UNorm_SRGB:      return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case TextureFormat::BC2_UNorm:           return VK_FORMAT_BC2_UNORM_BLOCK;
    case TextureFormat::BC2_UNorm_SRGB:      return VK_FORMAT_BC2_SRGB_BLOCK;
    case TextureFormat::BC3_UNorm:           return VK_FORMAT_BC3_UNORM_BLOCK;
    case TextureFormat::BC3_UNorm_SRGB:      return VK_FORMAT_BC3_SRGB_BLOCK;
    case TextureFormat::BC4_UNorm:           return VK_FORMAT_BC4_UNORM_BLOCK;
    case TextureFormat::BC4_SNorm:           return VK_FORMAT_BC4_SNORM_BLOCK;
    case TextureFormat::BC5_UNorm:           return VK_FORMAT_BC5_UNORM_BLOCK;
    case TextureFormat::BC5_SNorm:           return VK_FORMAT_BC5_SNORM_BLOCK;
    case TextureFormat::BC6H_UFloat:         return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case TextureFormat::BC6H_SFloat:         return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case TextureFormat::BC7_UNorm:           return VK_FORMAT_BC7_UNORM_BLOCK;
    case TextureFormat::BC7_UNorm_SRGB:      return VK_FORMAT_BC7_SRGB_BLOCK;

    default:                                 return VK_FORMAT_UNDEFINED;
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
    case VK_FORMAT_R16_SFLOAT:                      return TextureFormat::R16_Float;
    case VK_FORMAT_R8G8_UNORM:                      return TextureFormat::RG8_UNorm;
    case VK_FORMAT_R8G8_SNORM:                      return TextureFormat::RG8_SNorm;
    case VK_FORMAT_R8G8_UINT:                       return TextureFormat::RG8_UInt;
    case VK_FORMAT_R8G8_SINT:                       return TextureFormat::RG8_SInt;

    case VK_FORMAT_R32_UINT:                        return TextureFormat::R32_UInt;
    case VK_FORMAT_R32_SINT:                        return TextureFormat::R32_SInt;
    case VK_FORMAT_R32_SFLOAT:                      return TextureFormat::R32_Float;
    case VK_FORMAT_R16G16_UNORM:                    return TextureFormat::RG16_UNorm;
    case VK_FORMAT_R16G16_SNORM:                    return TextureFormat::RG16_SNorm;
    case VK_FORMAT_R16G16_UINT:                     return TextureFormat::RG16_UInt;
    case VK_FORMAT_R16G16_SINT:                     return TextureFormat::RG16_SInt;
    case VK_FORMAT_R16G16_SFLOAT:                   return TextureFormat::RG16_Float;

    case VK_FORMAT_R8G8B8A8_UNORM:                  return TextureFormat::RGBA8_UNorm;
    case VK_FORMAT_R8G8B8A8_SRGB:                   return TextureFormat::RGBA8_UNorm_SRGB;
    case VK_FORMAT_R8G8B8A8_SNORM:                  return TextureFormat::RGBA8_SNorm;
    case VK_FORMAT_R8G8B8A8_UINT:                   return TextureFormat::RGBA8_UInt;
    case VK_FORMAT_R8G8B8A8_SINT:                   return TextureFormat::RGBA8_SInt;
    case VK_FORMAT_B8G8R8A8_UNORM:                  return TextureFormat::BGRA8_UNorm;
    case VK_FORMAT_B8G8R8A8_SRGB:                   return TextureFormat::BGRA8_UNorm_SRGB;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:        return TextureFormat::RGB10A2_UNorm;
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:         return TextureFormat::RGB10A2_UInt;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:         return TextureFormat::RG11B10_Float;

    case VK_FORMAT_R32G32_UINT:                     return TextureFormat::RG32_UInt;
    case VK_FORMAT_R32G32_SINT:                     return TextureFormat::RG32_SInt;
    case VK_FORMAT_R32G32_SFLOAT:                   return TextureFormat::RG32_Float;
    case VK_FORMAT_R16G16B16A16_UNORM:              return TextureFormat::RGBA16_UNorm;
    case VK_FORMAT_R16G16B16A16_SNORM:              return TextureFormat::RGBA16_SNorm;
    case VK_FORMAT_R16G16B16A16_UINT:               return TextureFormat::RGBA16_UInt;
    case VK_FORMAT_R16G16B16A16_SINT:               return TextureFormat::RGBA16_SInt;
    case VK_FORMAT_R16G16B16A16_SFLOAT:             return TextureFormat::RGBA16_Float;

    case VK_FORMAT_R32G32B32_UINT:                  return TextureFormat::RGB32_UInt;
    case VK_FORMAT_R32G32B32_SINT:                  return TextureFormat::RGB32_SInt;
    case VK_FORMAT_R32G32B32_SFLOAT:                return TextureFormat::RGB32_Float;
    case VK_FORMAT_R32G32B32A32_UINT:               return TextureFormat::RGBA32_UInt;
    case VK_FORMAT_R32G32B32A32_SINT:               return TextureFormat::RGBA32_SInt;
    case VK_FORMAT_R32G32B32A32_SFLOAT:             return TextureFormat::RGBA32_Float;

    case VK_FORMAT_D16_UNORM:                       return TextureFormat::D16_UNorm;
    case VK_FORMAT_D32_SFLOAT:                      return TextureFormat::D32_Float;
    case VK_FORMAT_D24_UNORM_S8_UINT:               return TextureFormat::D24_UNorm_S8_UInt;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:              return TextureFormat::D32_Float_S8_UInt;

    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:            return TextureFormat::BC1_UNorm;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:             return TextureFormat::BC1_UNorm_SRGB;
    case VK_FORMAT_BC2_UNORM_BLOCK:                 return TextureFormat::BC2_UNorm;
    case VK_FORMAT_BC2_SRGB_BLOCK:                  return TextureFormat::BC2_UNorm_SRGB;
    case VK_FORMAT_BC3_UNORM_BLOCK:                 return TextureFormat::BC3_UNorm;
    case VK_FORMAT_BC3_SRGB_BLOCK:                  return TextureFormat::BC3_UNorm_SRGB;
    case VK_FORMAT_BC4_UNORM_BLOCK:                 return TextureFormat::BC4_UNorm;
    case VK_FORMAT_BC4_SNORM_BLOCK:                 return TextureFormat::BC4_SNorm;
    case VK_FORMAT_BC5_UNORM_BLOCK:                 return TextureFormat::BC5_UNorm;
    case VK_FORMAT_BC5_SNORM_BLOCK:                 return TextureFormat::BC5_SNorm;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:               return TextureFormat::BC6H_UFloat;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:               return TextureFormat::BC6H_SFloat;
    case VK_FORMAT_BC7_UNORM_BLOCK:                 return TextureFormat::BC7_UNorm;
    case VK_FORMAT_BC7_SRGB_BLOCK:                  return TextureFormat::BC7_UNorm_SRGB;

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

}