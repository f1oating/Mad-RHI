#pragma once

#include <Mad-RHI/Common.h>
#include <cstdint>

namespace mad::rhi {

enum class ResourceUsage
{
    Immutable,
    Default,
    Dynamic,
    Readback,
};

enum ResourceBindFlags : uint32_t
{
    BindNone         = 0,
    VertexBuffer     = 1 << 0,
    IndexBuffer      = 1 << 1,
    UniformBuffer    = 1 << 2,
    ShaderResource   = 1 << 3,
    RenderTarget     = 1 << 4,
    DepthStencil     = 1 << 5,
    UnorderedAccess  = 1 << 6,
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
    ResourceBindFlags BindFlags = BindNone;
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
    ResourceBindFlags BindFlags = BindNone;
    ResourceUsage Usage = ResourceUsage::Default;
    BufferMode Mode = BufferMode::Undefined;
    uint32_t ElementByteStride = 0;
};

enum class ResourceState
{
    Undefined,
    RenderTarget,
    ShaderResource,
    UnorderedAccess,
    DepthWrite,
    DepthRead,
    CopyDst,
    CopySrc,
    Present,
};

enum class TextureViewType
{
    ShaderResource,
    RenderTarget,
    DepthStencil,
    ReadOnlyDepthStencil,
    UnorderedAccess
};

struct TextureViewDesc
{
    TextureViewType Type = TextureViewType::ShaderResource;
    uint32_t MostDetailedMip = 0;
    uint32_t NumMipLevels = 0;
    uint32_t FirstArraySlice = 0;
    uint32_t NumArraySlices = 0;
};

enum class BufferViewType
{
    ShaderResource,
    UnorderedAccess,
};

struct BufferViewDesc
{
    BufferViewType Type = BufferViewType::ShaderResource;
    uint64_t ByteOffset = 0;
    uint64_t ByteWidth  = 0; 
};

enum class FilterType : uint8_t
{
    Point,
    Linear,
    Anisotropic,

    ComparisonPoint,
    ComparisonLinear,
    ComparisonAnisotropic,
};

enum class TextureAddressMode : uint8_t
{
    Wrap,
    Mirror,
    Clamp,
    MirrorOnce,
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

enum class SamplerFlags : uint8_t
{
    None                        = 0,
    Subsampled                  = 1 << 0,
    SubsampledCoarseReconstruct = 1 << 1,
};

struct SamplerDesc
{
    FilterType MinFilter = FilterType::Linear;
    FilterType MagFilter = FilterType::Linear;
    FilterType MipFilter = FilterType::Linear;
    TextureAddressMode AddressU = TextureAddressMode::Clamp;
    TextureAddressMode AddressV = TextureAddressMode::Clamp;
    TextureAddressMode AddressW = TextureAddressMode::Clamp;
    SamplerFlags Flags = SamplerFlags::None;
    float MipLodBias = 0.0f;
    uint32_t MaxAnisotropy = 0;
    CompareOp Compare = CompareOp::Never;
    float MinLod = 0.0f;
    float MaxLod = 3.402823466e+38f;
};

class Texture : public Object
{
public:
    virtual ~Texture() = default;

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
    RefPtr<Texture> Texture;
    ResourceState   NewState;
    uint32_t        BaseMip    = 0;
    uint32_t        MipCount   = 0;
    uint32_t        BaseSlice  = 0;
    uint32_t        SliceCount = 0;
};

struct BufferBarrier
{
    RefPtr<Buffer>  Buffer;
    ResourceState   NewState;
};

class TextureView : public Object
{
public:
    virtual ~TextureView() = default;

};

class BufferView : public Object
{
public:
    virtual ~BufferView() = default;

};

}