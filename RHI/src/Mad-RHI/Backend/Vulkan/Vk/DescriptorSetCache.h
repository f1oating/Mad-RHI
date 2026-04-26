#pragma once

#include <volk/volk.h>
#include <unordered_map>
#include <vector>

namespace mad::rhi::vk {

struct DescriptorSlot 
{
    VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    bool Valid = false;
    VkBuffer Buffer = nullptr;
    VkDeviceSize BufferOffset = 0;
    VkDeviceSize BufferRange = 0;
    VkImageView ImageView = nullptr;
    VkSampler Sampler = nullptr;
    VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

class DescriptorSetCache {
public:
    void Init(VkDevice device, uint32_t maxSetsPerPool = 256);
    void Shutdown();
    void Reset();

    VkDescriptorSet GetOrCreate(VkDescriptorSetLayout layout,
        uint64_t hash, const DescriptorSlot* slots, uint32_t slotCount);

private:
    VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
    void GrowPool();

private:
    VkDevice m_Device = nullptr;
    uint32_t m_MaxSetsPerPool = 0;

    struct Pool 
    {
        VkDescriptorPool Handle = nullptr;
        uint32_t Allocated = 0;
    };
    std::vector<Pool> m_Pools;

    std::unordered_map<uint64_t, VkDescriptorSet> m_Cache;

};

}