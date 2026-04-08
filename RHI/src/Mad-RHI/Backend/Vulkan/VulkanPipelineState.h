#pragma once

#include "Mad-RHI/PipelineState.h"
#include <volk/volk.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace mad::rhi {

class VulkanDevice;

struct VulkanReflectedShaderResource
{
    std::string Name;
    uint32_t Set;
    uint32_t Binding;
    VkDescriptorType Type;
    VkShaderStageFlags Stage;
    uint32_t ArraySize;
};

struct VulkanReflectedShaderSet
{
    uint32_t Index;
    std::vector<VulkanReflectedShaderResource> Resources;
};

class VulkanShaderResourceReflection
{
public:
    void Merge(const VulkanShaderResourceReflection* other);    

    void AddStage(const uint32_t* byteCode, uint64_t size, VkShaderStageFlagBits stage);

    const VulkanReflectedShaderResource* Find(std::string name);

    std::vector<const VulkanReflectedShaderResource*> GetSet(uint32_t index);
    uint32_t GetMaxSet() { return m_MaxSet; }

private:
    uint32_t m_MaxSet = 0;
    std::unordered_map<std::string, VulkanReflectedShaderResource> m_Resources;

};

class VulkanShader : public ObjectBase<Shader>
{
protected:
    ~VulkanShader();
    
public:
    VulkanShader(VkDevice device, const uint32_t* byteCode, uint64_t size);

    virtual ShaderType GetType() override;

    VkShaderModule GetShaderModule() { return m_ShaderModule; }
    const VulkanShaderResourceReflection& GetShaderResourceReflection() { return m_ResourceReflection; }
    const std::vector<VkVertexInputAttributeDescription>& GetVertexInputAttributes() { return m_VertexAttributes; }
    const VkVertexInputBindingDescription& GetVertexInputBinding() { return m_VertexBinding; }

private:
    VkDevice m_Device = nullptr;

    VkShaderModule m_ShaderModule = nullptr;
    VkShaderStageFlagBits m_ShaderStage = VK_SHADER_STAGE_VERTEX_BIT;

    VulkanShaderResourceReflection m_ResourceReflection;
    std::vector<VkVertexInputAttributeDescription> m_VertexAttributes;
    VkVertexInputBindingDescription m_VertexBinding{};

};

class VulkanGraphicsPipelineState : public ObjectBase<GraphicsPipelineState>
{
protected:
    ~VulkanGraphicsPipelineState();

public:
    VulkanGraphicsPipelineState(VkDevice device, const GraphicsPipelineDesc& desc,
        VulkanDevice* context);

    VkPipeline GetPipeline() { return m_Pipeline; }
    VkPipelineLayout GetPipelineLayout() { return m_Layout; }

    VkDescriptorSetLayout GetSetLayout(uint32_t set) { return m_SetLayouts[set]; }
    uint32_t GetSetCount() { return m_SetLayouts.size(); }

    const VulkanShaderResourceReflection& GetReflection() { return m_MergedReflection; }

private:
    void CreateLayout();
    void CreatePipeline();

private:
    VkDevice m_Device = nullptr;
    GraphicsPipelineDesc m_Desc;

    VkPipeline m_Pipeline = nullptr;
    VkPipelineLayout m_Layout = nullptr;
    std::vector<VkDescriptorSetLayout> m_SetLayouts;

    VulkanShaderResourceReflection m_MergedReflection;

    VulkanDevice* m_Context;

};

inline ShaderType FromVkShaderStage(VkShaderStageFlagBits shaderStage)
{
    if (shaderStage & VK_SHADER_STAGE_VERTEX_BIT) return ShaderType::VERTEX;
    if (shaderStage & VK_SHADER_STAGE_FRAGMENT_BIT) return ShaderType::FRAGMENT;
}

inline VkPolygonMode ToVkPolygonMode(PolygonMode mode)
{
    switch (mode)
    {
    case PolygonMode::Fill:  return VK_POLYGON_MODE_FILL;
    case PolygonMode::Line:  return VK_POLYGON_MODE_LINE;
    case PolygonMode::Point: return VK_POLYGON_MODE_POINT;
    default:                 return VK_POLYGON_MODE_FILL;
    }
}

inline VkCullModeFlags ToVkCullMode(CullMode mode)
{
    switch (mode)
    {
    case CullMode::None:  return VK_CULL_MODE_NONE;
    case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
    case CullMode::Back:  return VK_CULL_MODE_BACK_BIT;
    default:              return VK_CULL_MODE_NONE;
    }
}

inline VkFrontFace ToVkFrontFace(FrontFace face)
{
    switch (face)
    {
    case FrontFace::CCW: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case FrontFace::CW:  return VK_FRONT_FACE_CLOCKWISE;
    default:             return VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
    default:                      return VK_COMPARE_OP_ALWAYS;
    }
}

inline VkStencilOp ToVkStencilOp(StencilOp op)
{
    switch (op)
    {
    case StencilOp::Keep:      return VK_STENCIL_OP_KEEP;
    case StencilOp::Zero:      return VK_STENCIL_OP_ZERO;
    case StencilOp::Replace:   return VK_STENCIL_OP_REPLACE;
    case StencilOp::IncrClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case StencilOp::DecrClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case StencilOp::Invert:    return VK_STENCIL_OP_INVERT;
    case StencilOp::IncrWrap:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case StencilOp::DecrWrap:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    default:                   return VK_STENCIL_OP_KEEP;
    }
}

inline VkStencilOpState ToVkStencilOpState(const StencilOpDesc& desc)
{
    VkStencilOpState s{};
    s.failOp      = ToVkStencilOp(desc.Fail);
    s.passOp      = ToVkStencilOp(desc.Pass);
    s.depthFailOp = ToVkStencilOp(desc.DepthFail);
    s.compareOp   = ToVkCompareOp(desc.Compare);
    s.compareMask = desc.CompareMask;
    s.writeMask   = desc.WriteMask;
    s.reference   = desc.Reference;
    return s;
}

inline VkBlendFactor ToVkBlendFactor(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::Zero:               return VK_BLEND_FACTOR_ZERO;
    case BlendFactor::One:                return VK_BLEND_FACTOR_ONE;
    case BlendFactor::SrcColor:           return VK_BLEND_FACTOR_SRC_COLOR;
    case BlendFactor::OneMinusSrcColor:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DstColor:           return VK_BLEND_FACTOR_DST_COLOR;
    case BlendFactor::OneMinusDstColor:   return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case BlendFactor::SrcAlpha:           return VK_BLEND_FACTOR_SRC_ALPHA;
    case BlendFactor::OneMinusSrcAlpha:   return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DstAlpha:           return VK_BLEND_FACTOR_DST_ALPHA;
    case BlendFactor::OneMinusDstAlpha:   return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case BlendFactor::ConstColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case BlendFactor::OneMinusConstColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case BlendFactor::ConstAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case BlendFactor::OneMinusConstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    default:                              return VK_BLEND_FACTOR_ZERO;
    }
}

inline VkBlendOp ToVkBlendOp(BlendOp op)
{
    switch (op)
    {
    case BlendOp::Add:             return VK_BLEND_OP_ADD;
    case BlendOp::Subtract:        return VK_BLEND_OP_SUBTRACT;
    case BlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case BlendOp::Min:             return VK_BLEND_OP_MIN;
    case BlendOp::Max:             return VK_BLEND_OP_MAX;
    default:                       return VK_BLEND_OP_ADD;
    }
}

inline VkColorComponentFlags ToVkColorWriteMask(uint8_t mask)
{
    VkColorComponentFlags result = 0;
    if (mask & COLOR_WRITE_R) result |= VK_COLOR_COMPONENT_R_BIT;
    if (mask & COLOR_WRITE_G) result |= VK_COLOR_COMPONENT_G_BIT;
    if (mask & COLOR_WRITE_B) result |= VK_COLOR_COMPONENT_B_BIT;
    if (mask & COLOR_WRITE_A) result |= VK_COLOR_COMPONENT_A_BIT;
    return result;
}

inline VkPrimitiveTopology ToVkPrimitiveTopology(PrimitiveTopology topology)
{
    switch (topology)
    {
    case PrimitiveTopology::PointList:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case PrimitiveTopology::LineList:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case PrimitiveTopology::LineStrip:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case PrimitiveTopology::TriangleList:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case PrimitiveTopology::TriangleFan:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case PrimitiveTopology::PatchList:     return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default:                               return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

inline VkSampleCountFlagBits ToVkSampleCount(uint32_t count)
{
    switch (count)
    {
    case 1:  return VK_SAMPLE_COUNT_1_BIT;
    case 2:  return VK_SAMPLE_COUNT_2_BIT;
    case 4:  return VK_SAMPLE_COUNT_4_BIT;
    case 8:  return VK_SAMPLE_COUNT_8_BIT;
    case 16: return VK_SAMPLE_COUNT_16_BIT;
    case 32: return VK_SAMPLE_COUNT_32_BIT;
    case 64: return VK_SAMPLE_COUNT_64_BIT;
    default: return VK_SAMPLE_COUNT_1_BIT;
    }
}

};