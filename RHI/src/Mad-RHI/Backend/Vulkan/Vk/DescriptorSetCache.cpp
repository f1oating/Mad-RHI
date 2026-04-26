#include "Mad-RHI/Backend/Vulkan/Vk/DescriptorSetCache.h"

#include <vector>

namespace mad::rhi::vk {

static const VkDescriptorPoolSize kPoolSizeRatios[] = 
{
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          10 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,  10 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,          10 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  10 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,           10 },
    { VK_DESCRIPTOR_TYPE_SAMPLER,                 10 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,            5 },
};

void DescriptorSetCache::Init(VkDevice device, uint32_t maxSetsPerPool)
{
    m_Device = device;
    m_MaxSetsPerPool = maxSetsPerPool;
    GrowPool();
}

void DescriptorSetCache::Shutdown()
{
    for (auto& p : m_Pools)
    {
        vkDestroyDescriptorPool(m_Device, p.Handle, nullptr);
    }
    m_Pools.clear();
    m_Cache.clear();
}

void DescriptorSetCache::Reset()
{
    for (auto& p : m_Pools) 
    {
        vkResetDescriptorPool(m_Device, p.Handle, 0);
        p.Allocated = 0;
    }
    m_Cache.clear();
}

VkDescriptorSet DescriptorSetCache::GetOrCreate(
    VkDescriptorSetLayout layout, uint64_t hash,
    const DescriptorSlot* slots, uint32_t slotCount)
{
    auto it = m_Cache.find(hash);
    if (it != m_Cache.end())
        return it->second;

    VkDescriptorSet set = Allocate(layout);

    std::vector<VkDescriptorBufferInfo> bufInfos;
    std::vector<VkDescriptorImageInfo> imgInfos;
    std::vector<VkWriteDescriptorSet> writes;
    bufInfos.reserve(slotCount);
    imgInfos.reserve(slotCount);

    for (uint32_t b = 0; b < slotCount; ++b) 
    {
        const auto& slot = slots[b];
        if (!slot.Valid) continue;

        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.dstSet = set;
        w.dstBinding = b;
        w.descriptorCount = 1;
        w.descriptorType = slot.Type;

        const bool isBuffer = (slot.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            slot.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
            slot.Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
            slot.Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);

        if (isBuffer) 
        {
            bufInfos.push_back({ slot.Buffer, slot.BufferOffset, slot.BufferRange });
            w.pBufferInfo = &bufInfos.back();
        } else 
        {
            imgInfos.push_back({ slot.Sampler, slot.ImageView, slot.ImageLayout });
            w.pImageInfo = &imgInfos.back();
        }
        writes.push_back(w);
    }

    if (!writes.empty())
        vkUpdateDescriptorSets(m_Device, (uint32_t)writes.size(), writes.data(), 0, nullptr);

    m_Cache[hash] = set;
    return set;
}

void DescriptorSetCache::GrowPool()
{
    std::vector<VkDescriptorPoolSize> sizes;
    for (const auto& s : kPoolSizeRatios)
    {
        sizes.push_back({ s.type, s.descriptorCount * m_MaxSetsPerPool });
    }

    VkDescriptorPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.maxSets = m_MaxSetsPerPool;
    ci.poolSizeCount = (uint32_t)sizes.size();
    ci.pPoolSizes = sizes.data();

    Pool pool;
    vkCreateDescriptorPool(m_Device, &ci, nullptr, &pool.Handle);
    m_Pools.push_back(pool);
}

VkDescriptorSet DescriptorSetCache::Allocate(VkDescriptorSetLayout layout)
{
    if (m_Pools.back().Allocated >= m_MaxSetsPerPool)
        GrowPool();

    auto& pool = m_Pools.back();

    VkDescriptorSetAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    ai.descriptorPool = pool.Handle;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts = &layout;

    VkDescriptorSet set;
    vkAllocateDescriptorSets(m_Device, &ai, &set);
    pool.Allocated++;
    return set;
}

}