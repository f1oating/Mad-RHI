#pragma once

#include <volk/volk.h>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace mad::rhi::vk {

class DescriptorWriter
{
public:
    void WriteBuffer(uint32_t binding, VkDescriptorType type,
        VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
    void WriteImage(uint32_t binding, VkDescriptorType type,
        VkImageView view, VkSampler sampler, VkImageLayout layout);
    void UpdateSet(VkDevice device, VkDescriptorSet set);
    void Clear();

private:
    std::deque<VkDescriptorBufferInfo> m_BufferInfos;
    std::deque<VkDescriptorImageInfo> m_ImageInfos;
    std::vector<VkWriteDescriptorSet> m_Writes;

};

struct DescriptorSlot
{
    VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    bool Valid = false;

    uint64_t BufferId = 0;
    VkBuffer Buffer = nullptr;
    VkDeviceSize BufferOffset = 0;
    VkDeviceSize BufferRange = 0;

    uint64_t ImageViewId = 0;
    VkImageView ImageView = nullptr;
    uint64_t SamplerId = 0;
    VkSampler Sampler = nullptr;
    VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct DescriptorSet
{
    VkDescriptorSetLayout Layout = nullptr;
    std::vector<DescriptorSlot> Slots;
    bool Dirty = false;
};

class DescriptorSetAllocator;

class DescriptorState
{
public:
    void Init(const std::vector<VkDescriptorSetLayout>& setLayouts);

    void SetBuffer(uint32_t set, uint32_t binding, VkDescriptorType type,
        VkBuffer buffer, uint64_t bufferId, VkDeviceSize offset, VkDeviceSize range);

    void SetImage(uint32_t set, uint32_t binding, VkDescriptorType type,
        VkImageView view, uint64_t imageViewId, VkSampler sampler, uint64_t samplerId, VkImageLayout imageLayout);

    void UpdateAndBind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
        VkDevice device, DescriptorSetAllocator& allocator);

    void MarkAllDirty();
    bool HasAnySets() const { return !m_Sets.empty(); }

private:
    std::vector<DescriptorSet> m_Sets;
    DescriptorWriter m_Writer;

};

class DescriptorPool
{
public:
    static DescriptorPool* Create(VkDevice device, uint32_t maxSets,
        const std::vector<std::pair<VkDescriptorType, float>>& ratios);

    ~DescriptorPool();

    bool TryAllocate(VkDevice device, VkDescriptorSetLayout, VkDescriptorSet& out);
    void Reset(VkDevice device);
    bool IsFull() const { return m_Used >= m_MaxSets; }

private:
    VkDevice m_Device = nullptr;
    VkDescriptorPool m_Pool = nullptr;
    uint32_t m_MaxSets = 0;
    uint32_t m_Used = 0;

};

class DescriptorSetCache
{
public:
    ~DescriptorSetCache();

    bool Find(uint64_t hash, VkDescriptorSet& out);
    VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout);
    void Register(uint64_t hash, VkDescriptorSet set);
    void Reset(VkDevice device);
    void Touch(uint64_t timelineValue) { m_LastUsedTimelineValue = timelineValue; }
    bool IsUnused(uint64_t completedTimelineValue) const;

private:
    DescriptorPool* GetOrCreatePool(VkDevice device);

private:
    std::vector<DescriptorPool*> m_Pools;
    std::unordered_map<uint64_t, VkDescriptorSet> m_Cache;
    uint64_t m_LastUsedTimelineValue = UINT64_MAX;

};

class DescriptorSetAllocator
{
public:
    VkDescriptorSet FindOrAllocate(VkDevice device, VkDescriptorSetLayout,
        const DescriptorSet& state, bool& outCacheHit);

    void CommitSubmission(uint64_t value);

    void GC(uint64_t completedTimelineValue);

private:
    std::unordered_map<size_t, DescriptorSetCache> m_Caches;
    std::unordered_set<size_t> m_DirtyLayoutKeys;

};

}
