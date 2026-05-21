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
    Texture2DMS,
    Texture2DMSArray,
    Texture3D,
    TextureCube,
    TextureCubeArray,
};

enum class TextureFormat
{
    Unknown,

    R8_UNorm,   R8_SNorm,   R8_UInt,   R8_SInt,

    R16_UNorm,  R16_SNorm,  R16_UInt,  R16_SInt,  R16_Float,
    RG8_UNorm,  RG8_SNorm,  RG8_UInt,  RG8_SInt,

    R32_UInt,   R32_SInt,   R32_Float,
    RG16_UNorm, RG16_SNorm, RG16_UInt, RG16_SInt, RG16_Float,

    RGBA8_UNorm, RGBA8_UNorm_SRGB, RGBA8_SNorm, RGBA8_UInt, RGBA8_SInt,
    BGRA8_UNorm, BGRA8_UNorm_SRGB,
    RGB10A2_UNorm, RGB10A2_UInt, RG11B10_Float,

    RG32_UInt,   RG32_SInt,   RG32_Float,
    RGBA16_UNorm, RGBA16_SNorm, RGBA16_UInt, RGBA16_SInt, RGBA16_Float,

    RGB32_UInt,  RGB32_SInt,  RGB32_Float,
    RGBA32_UInt, RGBA32_SInt, RGBA32_Float,

    D16_UNorm, D32_Float, D24_UNorm_S8_UInt, D32_Float_S8_UInt,

    BC1_UNorm, BC1_UNorm_SRGB,
    BC2_UNorm, BC2_UNorm_SRGB,
    BC3_UNorm, BC3_UNorm_SRGB,
    BC4_UNorm, BC4_SNorm,
    BC5_UNorm, BC5_SNorm,
    BC6H_UFloat, BC6H_SFloat,
    BC7_UNorm, BC7_UNorm_SRGB,
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
    TextureFormat Format = TextureFormat::RGBA8_UNorm;
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