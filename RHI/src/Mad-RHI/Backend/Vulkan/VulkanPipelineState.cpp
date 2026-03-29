#include "Mad-RHI/Backend/Vulkan/VulkanPipelineState.h"
#include <spirv_reflect.h>

namespace mad::rhi {

VulkanShader::VulkanShader(VkDevice device, const uint32_t* byteCode, uint64_t size)
{
    m_Device = device;

    SpvReflectShaderModule spvModule;
    spvReflectCreateShaderModule(size, byteCode, &spvModule);
    m_ShaderStage = static_cast<VkShaderStageFlagBits>(spvModule.shader_stage);

    uint32_t count;
    spvReflectEnumerateDescriptorBindings(&spvModule, &count, nullptr);
    std::vector<SpvReflectDescriptorBinding*> bindings(count);
    spvReflectEnumerateDescriptorBindings(&spvModule, &count, bindings.data());

    for (auto* binding : bindings)
    {
        m_Bindings.push_back({
            binding->name,
            binding->set,
            binding->binding,
            static_cast<VkDescriptorType>(binding->descriptor_type),
            binding->count,
            binding->block.size
        });
    }

    spvReflectDestroyShaderModule(&spvModule);

    VkShaderModuleCreateInfo smci{};
    smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smci.codeSize = size * sizeof(uint32_t);
    smci.pCode = byteCode;
    vkCreateShaderModule(m_Device, &smci, nullptr, &m_ShaderModule);
}

VulkanShader::~VulkanShader()
{
    if (m_ShaderModule)
    {
        vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
    }
}

ShaderType VulkanShader::GetType()
{
    return FromVkShaderStage(m_ShaderStage);
}

}