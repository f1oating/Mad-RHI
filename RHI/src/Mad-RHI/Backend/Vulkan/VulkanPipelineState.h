#pragma once

#include "Mad-RHI/PipelineState.h"
#include <volk/volk.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace mad::rhi {

inline ShaderType FromVkShaderStage(VkShaderStageFlagBits shaderStage)
{
    if (shaderStage & VK_SHADER_STAGE_VERTEX_BIT) return ShaderType::VERTEX;
    if (shaderStage & VK_SHADER_STAGE_FRAGMENT_BIT) return ShaderType::FRAGMENT;
}

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

    std::vector<const VulkanReflectedShaderResource*> GetSet(uint32_t index) const;

private:
    std::unordered_map<std::string, VulkanReflectedShaderResource> m_Resources;

};

class VulkanShader : public ObjectBase<Shader>
{
protected:
    ~VulkanShader();
    
public:
    VulkanShader(VkDevice device, const uint32_t* byteCode, uint64_t size);

    virtual ShaderType GetType() override;

private:
    VkDevice m_Device = nullptr;

    VkShaderModule m_ShaderModule = nullptr;
    VkShaderStageFlagBits m_ShaderStage = VK_SHADER_STAGE_VERTEX_BIT;

    VulkanShaderResourceReflection m_ResourceReflection;
    std::vector<VkVertexInputAttributeDescription> m_VertexAttributes;

};

class VulkanGraphicsPipeline : public ObjectBase<GraphicsPipeline>
{
protected:
    ~VulkanGraphicsPipeline();

public:
    VulkanGraphicsPipeline();

};

};