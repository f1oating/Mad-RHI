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
    BindNone             = 0,
    VertexBuffer     = 1 << 0,
    IndexBuffer      = 1 << 1,
    UniformBuffer    = 1 << 2,
    ShaderResource   = 1 << 3,
    StreamOutput     = 1 << 4,
    RenderTarget     = 1 << 5,
    DepthStencil     = 1 << 6,
    UnorderedAccess  = 1 << 7,
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
    const char*         Name        = nullptr;
    TextureDimension    Dimension   = TextureDimension::Texture2D;
    uint32_t            Width       = 0;
    uint32_t            Height      = 0;
    union 
    { 
        uint32_t        ArraySize   = 1;
        uint32_t        Depth; 
    };
    TextureFormat       Format      = TextureFormat::RGBA8_UNorm;
    uint32_t            MipLevels   = 1;
    uint32_t            SampleCount = 1;
    ResourceBindFlags   BindFlags   = BindNone;
    ResourceUsage       Usage       = ResourceUsage::Default;
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
    uint64_t            Size              = 0;
    ResourceBindFlags   BindFlags         = BindNone;
    ResourceUsage       Usage             = ResourceUsage::Default;
    BufferMode          Mode              = BufferMode::Undefined;
    uint32_t            ElementByteStride = 0;
};

class Texture : public Object
{
public:
    virtual ~Texture() = default;

};

class Buffer : public Object
{
public:
    virtual ~Buffer() = default;

};

}