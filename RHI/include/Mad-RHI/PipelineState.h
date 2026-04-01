#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/Resource.h"
#include <vector>

namespace mad::rhi {
    
enum class ShaderType
{
    VERTEX, 
    FRAGMENT,
};

class Shader : public Object
{
public:
    virtual ~Shader() = default;

    virtual ShaderType GetType() = 0;

};

enum class PolygonMode
{
    Fill, Line, Point,
};

enum class CullMode
{
    None, Front, Back,
};

enum class FrontFace
{
    CCW, CW,
};

struct RasterizationDesc
{
    PolygonMode     Polygon             = PolygonMode::Fill;
    CullMode        Cull                = CullMode::Back;
    FrontFace       Face                = FrontFace::CCW;
    bool            DepthClampEnable    = false;
    bool            DepthBiasEnable     = false;
    float           DepthBiasConstant   = 0.0f;
    float           DepthBiasSlope      = 0.0f;
    float           LineWidth           = 1.0f;
};

enum class CompareOp
{
    Never, Less, Equal, LessEqual,
    Greater, NotEqual, GreaterEqual, Always,
};

enum class StencilOp
{
    Keep, Zero, Replace, IncrClamp, DecrClamp,
    Invert, IncrWrap, DecrWrap
};

struct StencilOpDesc 
{
    StencilOp   Fail          = StencilOp::Keep;
    StencilOp   Pass          = StencilOp::Keep;
    StencilOp   DepthFail     = StencilOp::Keep;
    CompareOp   Compare       = CompareOp::Always;
    uint32_t    CompareMask   = 0;
    uint32_t    WriteMask     = 0;
    uint32_t    Reference     = 0;
};

struct DepthStencilDesc 
{
    bool            DepthTestEnable   = true;
    bool            DepthWriteEnable  = true;
    CompareOp       DepthCompareOp    = CompareOp::Less;
    bool            StencilTestEnable = false;
    StencilOpDesc   Front;
    StencilOpDesc   Back;
};

enum class BlendFactor 
{
    Zero, One, SrcColor, OneMinusSrcColor,
    DstColor, OneMinusDstColor,
    SrcAlpha, OneMinusSrcAlpha,
    DstAlpha, OneMinusDstAlpha,
    ConstColor, OneMinusConstColor,
    ConstAlpha, OneMinusConstAlpha,
};

enum class BlendOp 
{ 
    Add, Subtract, ReverseSubtract, Min, Max 
};

enum ColorWriteMask : uint8_t 
{
    COLOR_WRITE_R    = 0x1,
    COLOR_WRITE_G    = 0x2,
    COLOR_WRITE_B    = 0x4,
    COLOR_WRITE_A    = 0x8,
    COLOR_WRITE_All  = 0xF,
};

struct ColorAttachmentBlend 
{
    bool        BlendEnable    = false;
    BlendFactor SrcColorFactor = BlendFactor::SrcAlpha;
    BlendFactor DstColorFactor = BlendFactor::OneMinusSrcAlpha;
    BlendOp     ColorOp        = BlendOp::Add;
    BlendFactor SrcAlphaFactor = BlendFactor::One;
    BlendFactor DstAlphaFactor = BlendFactor::Zero;
    BlendOp     AlphaOp        = BlendOp::Add;
    uint8_t     WriteMask      = COLOR_WRITE_All;
};

struct RenderingDesc 
{
    std::vector<TextureFormat>  ColorFormats;
    TextureFormat               DepthFormat   = TextureFormat::Unknown;
    TextureFormat               StencilFormat = TextureFormat::Unknown;
    uint32_t                    SampleCount   = 1;
};

enum class PrimitiveTopology 
{
    PointList, LineList, LineStrip,
    TriangleList, TriangleStrip, TriangleFan,
    PatchList,
};

struct GraphicsPipelineDesc 
{
    RefPtr<Shader>                      VertexShader = nullptr;
    RefPtr<Shader>                      FragmentShader = nullptr;
    PrimitiveTopology                   Topology = PrimitiveTopology::TriangleList;
    RasterizationDesc                   Rasterization;
    DepthStencilDesc                    DepthStencil;
    std::vector<ColorAttachmentBlend>   BlendAttachments;
    RenderingDesc                       Rendering;
};

class GraphicsPipelineState : public Object
{
public:
    virtual ~GraphicsPipelineState() = default;

};

}