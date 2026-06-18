#pragma once

#include "Mad-RHI/Resource.h"
#include <dxgi.h>
#include <directx/d3dx12.h>
#include <D3D12MemAlloc.h>

namespace mad::rhi {

class DX12Device;

class DX12Texture : public ObjectBase<Texture>
{
protected:
    ~DX12Texture();

public:
    DX12Texture(const TextureDesc& desc, DX12Device* context);
    DX12Texture(ID3D12Resource* res, const TextureDesc& desc, DX12Device* context);

    virtual RefPtr<TextureView> GetDefaultSRV() override;
    virtual RefPtr<TextureView> GetDefaultRTV() override;
    virtual RefPtr<TextureView> GetDefaultDSV() override;

    virtual const TextureDesc& GetDesc() override;
    virtual ResourceState GetCurrentResourceState() override;

private:
    DX12Device* m_Context = nullptr;
    TextureDesc m_Desc; 

    D3D12MA::Allocation* m_Allocation = nullptr;
    ID3D12Resource* m_Resource = nullptr;

};

class DX12Buffer : public ObjectBase<Buffer>
{
protected:
    ~DX12Buffer();

public:
    DX12Buffer(const BufferDesc& desc, DX12Device* context);

    virtual void* Map() override;
    virtual void Unmap() override;
        
    virtual const BufferDesc& GetDesc() override;
    virtual ResourceState GetCurrentResourceState() override;

private:
    DX12Device* m_Context = nullptr;
    BufferDesc m_Desc;

    D3D12MA::Allocation* m_Allocation = nullptr;
    ID3D12Resource* m_Resource = nullptr;

};

class DX12Sampler : public ObjectBase<Sampler>
{
protected:
    ~DX12Sampler();

public:
    DX12Sampler(const SamplerDesc& desc);

    virtual const SamplerDesc& GetDesc() override;

private:
    
};

class DX12TextureView : public ObjectBase<TextureView>
{
protected:
    ~DX12TextureView();

public:
    DX12TextureView(RefCounter* sharedCounter, const TextureViewDesc& desc);

private:

};

inline DXGI_FORMAT ToDXGIFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::Unknown:                        return DXGI_FORMAT_UNKNOWN;

    case TextureFormat::R8_UNorm:                       return DXGI_FORMAT_R8_UNORM;
    case TextureFormat::R8_SNorm:                       return DXGI_FORMAT_R8_SNORM;
    case TextureFormat::R8_UInt:                        return DXGI_FORMAT_R8_UINT;
    case TextureFormat::R8_SInt:                        return DXGI_FORMAT_R8_SINT;

    case TextureFormat::R16_UNorm:                      return DXGI_FORMAT_R16_UNORM;
    case TextureFormat::R16_SNorm:                      return DXGI_FORMAT_R16_SNORM;
    case TextureFormat::R16_UInt:                       return DXGI_FORMAT_R16_UINT;
    case TextureFormat::R16_SInt:                       return DXGI_FORMAT_R16_SINT;
    case TextureFormat::R16_SFloat:                     return DXGI_FORMAT_R16_FLOAT;
    case TextureFormat::R8G8_UNorm:                     return DXGI_FORMAT_R8G8_UNORM;
    case TextureFormat::R8G8_SNorm:                     return DXGI_FORMAT_R8G8_SNORM;
    case TextureFormat::R8G8_UInt:                      return DXGI_FORMAT_R8G8_UINT;
    case TextureFormat::R8G8_SInt:                      return DXGI_FORMAT_R8G8_SINT;

    case TextureFormat::R32_UInt:                       return DXGI_FORMAT_R32_UINT;
    case TextureFormat::R32_SInt:                       return DXGI_FORMAT_R32_SINT;
    case TextureFormat::R32_SFloat:                     return DXGI_FORMAT_R32_FLOAT;
    case TextureFormat::R16G16_UNorm:                   return DXGI_FORMAT_R16G16_UNORM;
    case TextureFormat::R16G16_SNorm:                   return DXGI_FORMAT_R16G16_SNORM;
    case TextureFormat::R16G16_UInt:                    return DXGI_FORMAT_R16G16_UINT;
    case TextureFormat::R16G16_SInt:                    return DXGI_FORMAT_R16G16_SINT;
    case TextureFormat::R16G16_SFloat:                  return DXGI_FORMAT_R16G16_FLOAT;

    case TextureFormat::R8G8B8A8_UNorm:                 return DXGI_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::R8G8B8A8_SRGB_UNorm:            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case TextureFormat::R8G8B8A8_SNorm:                 return DXGI_FORMAT_R8G8B8A8_SNORM;
    case TextureFormat::R8G8B8A8_UInt:                  return DXGI_FORMAT_R8G8B8A8_UINT;
    case TextureFormat::R8G8B8A8_SInt:                  return DXGI_FORMAT_R8G8B8A8_SINT;
    case TextureFormat::B8G8R8A8_UNorm:                 return DXGI_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::B8G8R8A8_SRGB_UNorm:            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    case TextureFormat::R32G32_UInt:                    return DXGI_FORMAT_R32G32_UINT;
    case TextureFormat::R32G32_SInt:                    return DXGI_FORMAT_R32G32_SINT;
    case TextureFormat::R32G32_SFloat:                  return DXGI_FORMAT_R32G32_FLOAT;
    case TextureFormat::R16G16B16A16_UNorm:             return DXGI_FORMAT_R16G16B16A16_UNORM;
    case TextureFormat::R16G16B16A16_SNorm:             return DXGI_FORMAT_R16G16B16A16_SNORM;
    case TextureFormat::R16G16B16A16_UInt:              return DXGI_FORMAT_R16G16B16A16_UINT;
    case TextureFormat::R16G16B16A16_SInt:              return DXGI_FORMAT_R16G16B16A16_SINT;
    case TextureFormat::R16G16B16A16_SFloat:            return DXGI_FORMAT_R16G16B16A16_FLOAT;

    case TextureFormat::R32G32B32_UInt:                 return DXGI_FORMAT_R32G32B32_UINT;
    case TextureFormat::R32G32B32_SInt:                 return DXGI_FORMAT_R32G32B32_SINT;
    case TextureFormat::R32G32B32_SFloat:               return DXGI_FORMAT_R32G32B32_FLOAT;
    case TextureFormat::R32G32B32A32_UInt:              return DXGI_FORMAT_R32G32B32A32_UINT;
    case TextureFormat::R32G32B32A32_SInt:              return DXGI_FORMAT_R32G32B32A32_SINT;
    case TextureFormat::R32G32B32A32_SFloat:            return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case TextureFormat::D16_UNorm:                      return DXGI_FORMAT_D16_UNORM;
    case TextureFormat::D32_SFloat:                     return DXGI_FORMAT_D32_FLOAT;
    case TextureFormat::D24_UNorm_S8_UInt:              return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case TextureFormat::D32_SFloat_S8_UInt:             return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

    default:                                            return DXGI_FORMAT_UNKNOWN;
    }
}

inline TextureFormat ToTextureFormat(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_UNKNOWN:                           return TextureFormat::Unknown;

    case DXGI_FORMAT_R8_UNORM:                          return TextureFormat::R8_UNorm;
    case DXGI_FORMAT_R8_SNORM:                          return TextureFormat::R8_SNorm;
    case DXGI_FORMAT_R8_UINT:                           return TextureFormat::R8_UInt;
    case DXGI_FORMAT_R8_SINT:                           return TextureFormat::R8_SInt;

    case DXGI_FORMAT_R16_UNORM:                         return TextureFormat::R16_UNorm;
    case DXGI_FORMAT_R16_SNORM:                         return TextureFormat::R16_SNorm;
    case DXGI_FORMAT_R16_UINT:                          return TextureFormat::R16_UInt;
    case DXGI_FORMAT_R16_SINT:                          return TextureFormat::R16_SInt;
    case DXGI_FORMAT_R16_FLOAT:                         return TextureFormat::R16_SFloat;
    case DXGI_FORMAT_R8G8_UNORM:                        return TextureFormat::R8G8_UNorm;
    case DXGI_FORMAT_R8G8_SNORM:                        return TextureFormat::R8G8_SNorm;
    case DXGI_FORMAT_R8G8_UINT:                         return TextureFormat::R8G8_UInt;
    case DXGI_FORMAT_R8G8_SINT:                         return TextureFormat::R8G8_SInt;

    case DXGI_FORMAT_R32_UINT:                          return TextureFormat::R32_UInt;
    case DXGI_FORMAT_R32_SINT:                          return TextureFormat::R32_SInt;
    case DXGI_FORMAT_R32_FLOAT:                         return TextureFormat::R32_SFloat;
    case DXGI_FORMAT_R16G16_UNORM:                      return TextureFormat::R16G16_UNorm;
    case DXGI_FORMAT_R16G16_SNORM:                      return TextureFormat::R16G16_SNorm;
    case DXGI_FORMAT_R16G16_UINT:                       return TextureFormat::R16G16_UInt;
    case DXGI_FORMAT_R16G16_SINT:                       return TextureFormat::R16G16_SInt;
    case DXGI_FORMAT_R16G16_FLOAT:                      return TextureFormat::R16G16_SFloat;

    case DXGI_FORMAT_R8G8B8A8_UNORM:                    return TextureFormat::R8G8B8A8_UNorm;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:               return TextureFormat::R8G8B8A8_SRGB_UNorm;
    case DXGI_FORMAT_R8G8B8A8_SNORM:                    return TextureFormat::R8G8B8A8_SNorm;
    case DXGI_FORMAT_R8G8B8A8_UINT:                     return TextureFormat::R8G8B8A8_UInt;
    case DXGI_FORMAT_R8G8B8A8_SINT:                     return TextureFormat::R8G8B8A8_SInt;
    case DXGI_FORMAT_B8G8R8A8_UNORM:                    return TextureFormat::B8G8R8A8_UNorm;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:               return TextureFormat::B8G8R8A8_SRGB_UNorm;

    case DXGI_FORMAT_R32G32_UINT:                       return TextureFormat::R32G32_UInt;
    case DXGI_FORMAT_R32G32_SINT:                       return TextureFormat::R32G32_SInt;
    case DXGI_FORMAT_R32G32_FLOAT:                      return TextureFormat::R32G32_SFloat;
    case DXGI_FORMAT_R16G16B16A16_UNORM:                return TextureFormat::R16G16B16A16_UNorm;
    case DXGI_FORMAT_R16G16B16A16_SNORM:                return TextureFormat::R16G16B16A16_SNorm;
    case DXGI_FORMAT_R16G16B16A16_UINT:                 return TextureFormat::R16G16B16A16_UInt;
    case DXGI_FORMAT_R16G16B16A16_SINT:                 return TextureFormat::R16G16B16A16_SInt;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:                return TextureFormat::R16G16B16A16_SFloat;

    case DXGI_FORMAT_R32G32B32_UINT:                    return TextureFormat::R32G32B32_UInt;
    case DXGI_FORMAT_R32G32B32_SINT:                    return TextureFormat::R32G32B32_SInt;
    case DXGI_FORMAT_R32G32B32_FLOAT:                   return TextureFormat::R32G32B32_SFloat;
    case DXGI_FORMAT_R32G32B32A32_UINT:                 return TextureFormat::R32G32B32A32_UInt;
    case DXGI_FORMAT_R32G32B32A32_SINT:                 return TextureFormat::R32G32B32A32_SInt;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:                return TextureFormat::R32G32B32A32_SFloat;

    case DXGI_FORMAT_D16_UNORM:                         return TextureFormat::D16_UNorm;
    case DXGI_FORMAT_D32_FLOAT:                         return TextureFormat::D32_SFloat;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:                 return TextureFormat::D24_UNorm_S8_UInt;
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:              return TextureFormat::D32_SFloat_S8_UInt;

    default:                                            return TextureFormat::Unknown;
    }
}

inline D3D12_RESOURCE_DIMENSION ToDXGIResourceDimension(TextureDimension dimension)
{
    switch (dimension)
    {
    case TextureDimension::Texture1D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    case TextureDimension::Texture2D:
    case TextureDimension::TextureCube:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    case TextureDimension::Texture3D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    default:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    }
}

inline D3D12_RESOURCE_FLAGS ToD3D12ResourceFlags(uint8_t bindFlags)
{
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
    
    if (bindFlags & RESOURCE_BIND_RENDER_TARGET) flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (bindFlags & RESOURCE_BIND_DEPTH_STENCIL) flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    if (bindFlags & RESOURCE_BIND_UNORDERED_ACCESS) flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    return flags;
}

inline D3D12_RESOURCE_STATES ToD3D12ResourceState(ResourceState state)
{
    switch (state)
    {
    case ResourceState::Undefined:
        return D3D12_RESOURCE_STATE_COMMON;
    case ResourceState::VertexBuffer:
        return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    case ResourceState::IndexBuffer:
        return D3D12_RESOURCE_STATE_INDEX_BUFFER;
    case ResourceState::RenderTarget:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case ResourceState::ShaderResource:
        return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    case ResourceState::UnorderedAccess:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case ResourceState::DepthWrite:
        return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case ResourceState::DepthRead:
        return D3D12_RESOURCE_STATE_DEPTH_READ;
    case ResourceState::CopyDst:
        return D3D12_RESOURCE_STATE_COPY_DEST;
    case ResourceState::CopySrc:
        return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case ResourceState::Present:
        return D3D12_RESOURCE_STATE_PRESENT;
    default:
        return D3D12_RESOURCE_STATE_COMMON;
    }
}

}