#pragma once

#include <Mad-RHI/Common.h>
#include <cstdint>

namespace mad::rhi {

class TextureView;

enum class ResourceUsage
{
    Immutable,
    Default,
    Dynamic,
    Readback,
};

enum ResourceBind : uint8_t
{
    RESOURCE_BIND_NONE = 0,
    RESOURCE_BIND_VERTEX_BUFFER = 1 << 0,
    RESOURCE_BIND_INDEX_BUFFER = 1 << 1,
    RESOURCE_BIND_UNIFORM_BUFFER = 1 << 2,
    RESOURCE_BIND_SHADER_RESOURSE = 1 << 3,
    RESOURCE_BIND_RENDER_TARGET = 1 << 4,
    RESOURCE_BIND_DEPTH_STENCIL = 1 << 5,
    RESOURCE_BIND_UNORDERED_ACCESS = 1 << 6,
};

enum class TextureDimension
{
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture3D,
    TextureCube,
    TextureCubeArray,
};

enum class TextureFormat
{
    Unknown,

    R8_UNorm,   R8_SNorm,   R8_UInt,   R8_SInt,

    R16_UNorm,  R16_SNorm,  R16_UInt,  R16_SInt,  R16_SFloat,
    R8G8_UNorm,  R8G8_SNorm,  R8G8_UInt,  R8G8_SInt,

    R32_UInt,   R32_SInt,   R32_SFloat,
    R16G16_UNorm, R16G16_SNorm, R16G16_UInt, R16G16_SInt, R16G16_SFloat,

    R8G8B8A8_UNorm, R8G8B8A8_SRGB_UNorm, R8G8B8A8_SNorm, R8G8B8A8_UInt, R8G8B8A8_SInt,
    B8G8R8A8_UNorm, B8G8R8A8_SRGB_UNorm,

    R32G32_UInt,   R32G32_SInt,   R32G32_SFloat,
    R16G16B16A16_UNorm, R16G16B16A16_SNorm, R16G16B16A16_UInt, R16G16B16A16_SInt, R16G16B16A16_SFloat,

    R32G32B32_UInt,  R32G32B32_SInt,  R32G32B32_SFloat,
    R32G32B32A32_UInt, R32G32B32A32_SInt, R32G32B32A32_SFloat,

    D16_UNorm, D32_SFloat, D24_UNorm_S8_UInt, D32_SFloat_S8_UInt,

    BC1_RGBA_UNorm_Block, BC1_RGBA_SRGB_UNorm_Block,
    BC2_UNorm_Block, BC2_SRGB_UNorm_Block,
    BC3_UNorm_Block, BC3_SRGB_UNorm_Block,
    BC4_UNorm_Block, BC4_SNorm_Block,
    BC5_UNorm_Block, BC5_SNorm_Block,
    BC6H_UFloat_Block, BC6H_SFloat_Block,
    BC7_UNorm_Block, BC7_SRGB_UNorm_Block,
};

struct TextureDesc
{
    TextureDimension Dimension = TextureDimension::Texture2D;
    uint32_t Width = 0;
    uint32_t Height = 0;
    union 
    { 
        uint32_t ArraySize = 1;
        uint32_t Depth; 
    };
    TextureFormat Format = TextureFormat::R8G8B8A8_UNorm;
    uint32_t MipLevels = 1;
    uint32_t SampleCount = 1;
    uint8_t BindFlags = RESOURCE_BIND_NONE;
    ResourceUsage Usage = ResourceUsage::Default;
};

enum class BufferMode
{
    Undefined,
    Structured,
    Formatted,
    Raw,
};

struct BufferDesc 
{
    uint64_t Size = 0;
    uint8_t BindFlags = RESOURCE_BIND_NONE;
    ResourceUsage Usage = ResourceUsage::Default;
    BufferMode Mode = BufferMode::Undefined;
    uint32_t ElementByteStride = 0;
};

enum class ResourceState
{
    Undefined,
    VertexBuffer,
    IndexBuffer,
    RenderTarget,
    ShaderResource,
    UnorderedAccess,
    DepthWrite,
    DepthRead,
    CopyDst,
    CopySrc,
    Present,
};

enum class FilterType
{
    Nearest,
    Linear
};

enum class TextureAddressMode
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge
};

enum class CompareOp
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
};

enum class BorderColor
{
    FloatTransparentBlack,
    IntTransparentBlack,
    FloatOpaqueBlack,
    IntOpaqueBlack,
    FloatOpaqueWhite,
    IntOpaqueWhite
};

struct SamplerDesc
{
    FilterType MinFilter = FilterType::Linear;
    FilterType MagFilter = FilterType::Linear;
    FilterType MipFilter = FilterType::Linear;
    TextureAddressMode AddressU = TextureAddressMode::ClampToEdge;
    TextureAddressMode AddressV = TextureAddressMode::ClampToEdge;
    TextureAddressMode AddressW = TextureAddressMode::ClampToEdge;
    float MipLodBias = 0.0f;
    uint32_t MaxAnisotropy = 0;
    CompareOp Compare = CompareOp::Never;
    float MinLod = 0.0f;
    float MaxLod = 3.402823466e+38f;
    BorderColor Border = BorderColor::FloatTransparentBlack;
};

enum class TextureViewType 
{ 
    Undefined, 
    SRV, 
    RTV, 
    DSV, 
    UAV 
};

struct TextureViewDesc 
{
    TextureViewType ViewType = TextureViewType::Undefined;
    TextureFormat Format = TextureFormat::Unknown;
    uint32_t MostDetailedMip = 0;
    uint32_t NumMipLevels = 0;
    uint32_t FirstArraySlice = 0;
    uint32_t NumArraySlices = 0;
};

class Texture : public Object
{
public:
    virtual ~Texture() = default;

    virtual RefPtr<TextureView> GetDefaultSRV() = 0;
    virtual RefPtr<TextureView> GetDefaultRTV() = 0;
    virtual RefPtr<TextureView> GetDefaultDSV() = 0;

    virtual const TextureDesc& GetDesc() = 0;
    virtual ResourceState GetCurrentResourceState() = 0;

};

class Buffer : public Object
{
public:
    virtual ~Buffer() = default;

    virtual void* Map() = 0;
    virtual void Unmap() = 0;

    virtual const BufferDesc& GetDesc() = 0;
    virtual ResourceState GetCurrentResourceState() = 0;

};

class Sampler : public Object
{
public:
    virtual ~Sampler() = default;

    virtual const SamplerDesc& GetDesc() = 0;

};

struct TextureBarrier
{
    Texture* Texture;
    ResourceState NewState;
    uint32_t BaseMip = 0;
    uint32_t MipCount = 0;
    uint32_t BaseSlice = 0;
    uint32_t SliceCount = 0;
};

struct BufferBarrier
{
    Buffer* Buffer;
    ResourceState NewState;
};

class TextureView : public Object
{
public:
    virtual ~TextureView() = default;

};

}