#include "Mad-RHI/Backend/Vulkan/VulkanPipelineState.h"
#include <spirv_reflect.h>
#include <algorithm>

namespace mad::rhi {

static uint32_t SpvFormatSize(SpvReflectFormat fmt)
{
    switch (fmt)
    {
        case SPV_REFLECT_FORMAT_R32_UINT:
        case SPV_REFLECT_FORMAT_R32_SINT:
        case SPV_REFLECT_FORMAT_R32_SFLOAT:          return 4;
        case SPV_REFLECT_FORMAT_R32G32_UINT:
        case SPV_REFLECT_FORMAT_R32G32_SINT:
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:       return 8;
        case SPV_REFLECT_FORMAT_R32G32B32_UINT:
        case SPV_REFLECT_FORMAT_R32G32B32_SINT:
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:    return 12;
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return 16;
        default: return 0;
    }
}

void VulkanShaderResourceReflection::AddStage(const uint32_t* byteCode, uint64_t size, VkShaderStageFlagBits stage)
{
    SpvReflectShaderModule spvModule;
    spvReflectCreateShaderModule(size, byteCode, &spvModule);

    uint32_t descBindingCount;
    spvReflectEnumerateDescriptorBindings(&spvModule, &descBindingCount, nullptr);
    std::vector<SpvReflectDescriptorBinding*> bindings(descBindingCount);
    spvReflectEnumerateDescriptorBindings(&spvModule, &descBindingCount, bindings.data());

    for (auto* binding : bindings)
    {
        auto it = m_Resources.find(binding->name);

        if (it != m_Resources.end())
        {
            it->second.Stage |= stage;
        } else
        {
            m_Resources[binding->name] = VulkanReflectedShaderResource(
                binding->name,
                binding->set,
                binding->binding,
                static_cast<VkDescriptorType>(binding->descriptor_type),
                stage,
                binding->count
            );
        }
    }

    spvReflectDestroyShaderModule(&spvModule);
}

const VulkanReflectedShaderResource* VulkanShaderResourceReflection::Find(std::string name)
{
    auto it = m_Resources.find(name);
    return it != m_Resources.end() ? &it->second : nullptr;
}

std::vector<const VulkanReflectedShaderResource*> VulkanShaderResourceReflection::GetSet(uint32_t index) const
{
    std::vector<const VulkanReflectedShaderResource*> result;
    for (const auto& [name, res] : m_Resources)
    {
        if (res.Set == index) result.push_back(&res);
    }
    return result;
}

VulkanShader::VulkanShader(VkDevice device, const uint32_t* byteCode, uint64_t size)
{
    m_Device = device;

    SpvReflectShaderModule spvModule;
    spvReflectCreateShaderModule(size, byteCode, &spvModule);
    m_ShaderStage = static_cast<VkShaderStageFlagBits>(spvModule.shader_stage);

    m_ResourceReflection.AddStage(byteCode, size, m_ShaderStage);

    if (m_ShaderStage == VK_SHADER_STAGE_VERTEX_BIT)
    {
        uint32_t inputVariablesCount;
        spvReflectEnumerateInputVariables(&spvModule, &inputVariablesCount, nullptr);
        std::vector<SpvReflectInterfaceVariable*> inputVaribles;
        spvReflectEnumerateInputVariables(&spvModule, &inputVariablesCount, inputVaribles.data());

        std::sort(inputVaribles.begin(), inputVaribles.end(),
            [](const SpvReflectInterfaceVariable* a, const SpvReflectInterfaceVariable* b) {
                return a->location < b->location;
            });

        uint32_t offset = 0;
        for (auto* var : inputVaribles)
        {
            if (var->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
                continue;

            VkVertexInputAttributeDescription viad{};
            viad.location = var->location;
            viad.format = static_cast<VkFormat>(var->format);
            viad.binding = 0;
            viad.offset = offset;

            offset += SpvFormatSize(var->format);
            
            m_VertexAttributes.push_back(viad);
        }
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