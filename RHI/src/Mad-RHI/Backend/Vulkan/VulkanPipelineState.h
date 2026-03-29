#pragma once

#include "Mad-RHI/PipelineState.h"
#include <volk/volk.h>
#include <vector>
#include <string>

namespace mad::rhi {

inline ShaderType FromVkShaderStage(VkShaderStageFlagBits shaderStage)
{
    if (shaderStage & VK_SHADER_STAGE_VERTEX_BIT) return ShaderType::VERTEX;
    if (shaderStage & VK_SHADER_STAGE_FRAGMENT_BIT) return ShaderType::FRAGMENT;
}

struct VulkanShaderBinding
{
    std::string Name;
    uint32_t Set;
    uint32_t Binding;
    VkDescriptorType Type;
    uint32_t ArraySize;
    uint32_t BlockSize;
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

    std::vector<VulkanShaderBinding> m_Bindings;

};

};