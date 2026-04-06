#include "Mad-RHI/Backend/Vulkan/Vk/DescriptorSetAllocator.h"

#include <vector>

namespace mad::rhi::vk {

static const std::vector<VkDescriptorPoolSize> s_PoolSizes = 
{
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         10 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         10 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          10 },
    { VK_DESCRIPTOR_TYPE_SAMPLER,                10 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,           5 },
};

void DescriptorSetAllocator::Init(VkDevice device, uint32_t maxSets)
{
    m_Device = device;
    m_MaxSets = maxSets;
    CreatePool();
}

void DescriptorSetAllocator::Shutdown()
{
    if (m_Pool)
    {
        vkDestroyDescriptorPool(m_Device, m_Pool, nullptr);
    }
}

VkDescriptorSet DescriptorSetAllocator::Allocate(VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo dsai{};
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = m_Pool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = &layout;

    VkDescriptorSet set;
    vkAllocateDescriptorSets(m_Device, &dsai, &set);

    return set;
}

void DescriptorSetAllocator::Reset()
{
    vkResetDescriptorPool(m_Device, m_Pool, 0);
}

void DescriptorSetAllocator::CreatePool()
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto& s : s_PoolSizes)
    {
        poolSizes.push_back({ s.type, s.descriptorCount * m_MaxSets });
    }

    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.maxSets = m_MaxSets;
    dpci.poolSizeCount = poolSizes.size();
    dpci.pPoolSizes = poolSizes.data();

    vkCreateDescriptorPool(m_Device, &dpci, nullptr, &m_Pool);
}

}