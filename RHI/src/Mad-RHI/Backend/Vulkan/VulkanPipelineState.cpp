#include "Mad-RHI/Backend/Vulkan/VulkanPipelineState.h"
#include <spirv_reflect.h>
#include <algorithm>
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"

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

void VulkanShaderResourceReflection::Merge(const VulkanShaderResourceReflection* other)
{
    for (auto& [name, res] : other->m_Resources)
    {
        auto it = m_Resources.find(name);
        m_MaxSet = std::max(m_MaxSet, res.Set);
        if (it != m_Resources.end())
        {
            it->second.Stage |= res.Stage;
        } else
        {
            m_Resources[name] = res;
        }
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
        m_MaxSet = std::max(m_MaxSet, binding->set);

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

std::vector<const VulkanReflectedShaderResource*> VulkanShaderResourceReflection::GetSet(uint32_t index)
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
        std::vector<SpvReflectInterfaceVariable*> inputVaribles(inputVariablesCount);
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

        m_VertexBinding.binding   = 0;
        m_VertexBinding.stride    = offset;
        m_VertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
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

VulkanGraphicsPipelineState::VulkanGraphicsPipelineState(VkDevice device, const GraphicsPipelineDesc& desc)
{
    m_Device = device;

    VulkanShader* vertexShader = static_cast<VulkanShader*>(desc.VertexShader.Get());
    VulkanShader* fragmentShader = static_cast<VulkanShader*>(desc.FragmentShader.Get());

    VkPipelineShaderStageCreateInfo shaderStages[] = 
    {
        { 
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
            VK_SHADER_STAGE_VERTEX_BIT, vertexShader->GetShaderModule(),
            "main", nullptr 
        },
        { 
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
            VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader->GetShaderModule(),
            "main", nullptr 
        },
    };

    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &vertexShader->GetVertexInputBinding();
    vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexShader->GetVertexInputAttributes().size();
    vertexInputState.pVertexAttributeDescriptions = vertexShader->GetVertexInputAttributes().data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = ToVkPrimitiveTopology(desc.Topology);
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;

    const auto& rast = desc.Rasterization;

    VkPipelineRasterizationStateCreateInfo rasterizationState{};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = ToVkPolygonMode(rast.Polygon);
    rasterizationState.cullMode = ToVkCullMode(rast.Cull);
    rasterizationState.frontFace = ToVkFrontFace(rast.Face);
    rasterizationState.depthClampEnable = rast.DepthClampEnable;
    rasterizationState.depthBiasEnable = rast.DepthBiasEnable;
    rasterizationState.depthBiasConstantFactor = rast.DepthBiasConstant;
    rasterizationState.depthBiasSlopeFactor = rast.DepthBiasSlope;
    rasterizationState.lineWidth = rast.LineWidth;

    VkPipelineMultisampleStateCreateInfo multisampleState{};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = ToVkSampleCount(desc.Rendering.SampleCount);
    multisampleState.sampleShadingEnable = VK_FALSE;

    const auto& ds = desc.DepthStencil;

    VkPipelineDepthStencilStateCreateInfo depthStencilState{};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = ds.DepthTestEnable;
    depthStencilState.depthWriteEnable = ds.DepthWriteEnable;
    depthStencilState.depthCompareOp = ToVkCompareOp(ds.DepthCompareOp);
    depthStencilState.stencilTestEnable = ds.StencilTestEnable;
    depthStencilState.front = ToVkStencilOpState(ds.Front);
    depthStencilState.back = ToVkStencilOpState(ds.Back);
    depthStencilState.depthBoundsTestEnable = VK_FALSE;

    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
    blendAttachments.reserve(desc.BlendAttachments.size());

    for (const auto& src : desc.BlendAttachments)
    {
        VkPipelineColorBlendAttachmentState dst{};
        dst.blendEnable = src.BlendEnable;
        dst.srcColorBlendFactor = ToVkBlendFactor(src.SrcColorFactor);
        dst.dstColorBlendFactor = ToVkBlendFactor(src.DstColorFactor);
        dst.colorBlendOp = ToVkBlendOp(src.ColorOp);
        dst.srcAlphaBlendFactor = ToVkBlendFactor(src.SrcAlphaFactor);
        dst.dstAlphaBlendFactor = ToVkBlendFactor(src.DstAlphaFactor);
        dst.alphaBlendOp = ToVkBlendOp(src.AlphaOp);
        dst.colorWriteMask = ToVkColorWriteMask(src.WriteMask);
        blendAttachments.push_back(dst);
    }

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.attachmentCount = blendAttachments.size();
    colorBlendState.pAttachments = blendAttachments.data();

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    std::vector<VkFormat> colorFormats;
    colorFormats.reserve(desc.Rendering.ColorFormats.size());
    for (auto fmt : desc.Rendering.ColorFormats)
        colorFormats.push_back(ToVkFormat(fmt));

    VkPipelineRenderingCreateInfoKHR renderingCI{};
    renderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    renderingCI.colorAttachmentCount = (uint32_t)colorFormats.size();
    renderingCI.pColorAttachmentFormats = colorFormats.data();
    renderingCI.depthAttachmentFormat = ToVkFormat(desc.Rendering.DepthFormat);
    renderingCI.stencilAttachmentFormat = ToVkFormat(desc.Rendering.StencilFormat);

    VulkanShaderResourceReflection merged;
    merged.Merge(&vertexShader->GetShaderResourceReflection());
    merged.Merge(&fragmentShader->GetShaderResourceReflection());

    for (uint32_t setIndex = 0; setIndex <= merged.GetMaxSet(); setIndex++)
    {
        auto resources = merged.GetSet(setIndex);

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (auto& res : resources)
        {
            VkDescriptorSetLayoutBinding b{};
            b.binding = res->Binding;
            b.descriptorType = res->Type;
            b.descriptorCount = res->ArraySize;
            b.stageFlags = res->Stage;
            bindings.push_back(b);
        }

        VkDescriptorSetLayoutCreateInfo dslCI{};
        dslCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslCI.bindingCount = (uint32_t)bindings.size();
        dslCI.pBindings = bindings.data();

        VkDescriptorSetLayout layout;
        vkCreateDescriptorSetLayout(m_Device, &dslCI, nullptr, &layout);
        m_SetLayouts.push_back(layout);
    };

    VkPipelineLayoutCreateInfo layoutCI{};
    layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCI.setLayoutCount = (uint32_t)m_SetLayouts.size();
    layoutCI.pSetLayouts = m_SetLayouts.data();
    vkCreatePipelineLayout(m_Device, &layoutCI, nullptr, &m_Layout);

    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.pNext = &renderingCI;
    pipelineCI.stageCount = 2;
    pipelineCI.pStages = shaderStages;
    pipelineCI.pVertexInputState = &vertexInputState;
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = m_Layout;

    vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &m_Pipeline);
}

VulkanGraphicsPipelineState::~VulkanGraphicsPipelineState()
{
    if (m_Pipeline)
    {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    }
    if (m_Layout)
    {
        vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
    }
    for (auto layout : m_SetLayouts)
    {
        vkDestroyDescriptorSetLayout(m_Device, layout, nullptr);
    }
}

}