#include "Mad-RHI/Backend/Vulkan/Vk/DescriptorState.h"
#include <unordered_map>
#include <algorithm>

namespace mad::rhi::vk {

static uint64_t ComputeHash(const DescriptorSet& state)
{
    uint64_t hash = 14695981039346656037ULL;
    auto mix = [&](const void* data, size_t size) 
    {
        for (size_t i = 0; i < size; ++i) 
        {
            hash ^= reinterpret_cast<const uint8_t*>(data)[i];
            hash *= 1099511628211ULL;
        }
    };

    for (const auto& slot : state.Slots)
    {
        if (!slot.Valid) { uint64_t zero = 0; mix(&zero, 8); continue; }

        if (slot.Buffer != VK_NULL_HANDLE)
        {
            mix(&slot.BufferId,     8);
            mix(&slot.BufferOffset, 8);
            mix(&slot.BufferRange,  8);
        } else
        {
            mix(&slot.ImageViewId, 8);
            mix(&slot.SamplerId,   8);
            uint64_t layout = slot.ImageLayout;
            mix(&layout, 8);
        }
    }
    return hash;
}

static const std::vector<std::pair<VkDescriptorType, float>> DefaultRatios = 
{
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         0.25f  },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         0.125f },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4.0f   },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 0.125f },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2.0f   },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          2.0f   },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          0.125f },
    { VK_DESCRIPTOR_TYPE_SAMPLER,                2.0f   },
};

void DescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorType type,
    VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
{
    m_BufferInfos.push_back({ buffer, offset, range });

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstBinding = binding;
    w.descriptorCount = 1;
    w.descriptorType = type;
    w.pBufferInfo = &m_BufferInfos.back();
    m_Writes.push_back(w);
}

void DescriptorWriter::WriteImage(uint32_t binding, VkDescriptorType type,
    VkImageView view, VkSampler sampler, VkImageLayout imageLayout)
{
    m_ImageInfos.push_back({ sampler, view, imageLayout });

    VkWriteDescriptorSet w{};
    w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w.dstBinding = binding;
    w.descriptorCount = 1;
    w.descriptorType = type;
    w.pImageInfo = &m_ImageInfos.back();
    m_Writes.push_back(w);
}

void DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set)
{
    for (auto& w : m_Writes)
    {
        w.dstSet = set;
    }

    if (!m_Writes.empty())
    {
        vkUpdateDescriptorSets(device,
            static_cast<uint32_t>(m_Writes.size()), m_Writes.data(),
            0, nullptr);
    }
}

void DescriptorWriter::Clear()
{
    m_BufferInfos.clear();
    m_ImageInfos.clear();
    m_Writes.clear();
}

void DescriptorState::Init(const std::vector<VkDescriptorSetLayout>& setLayouts)
{
    m_Sets.resize(setLayouts.size());
    for (uint32_t i = 0; i < (uint32_t)setLayouts.size(); ++i)
    {
        m_Sets[i].Layout = setLayouts[i];
        m_Sets[i].Slots.resize(16);
        m_Sets[i].Dirty = false;
    }
}

void DescriptorState::SetBuffer(uint32_t set, uint32_t binding, VkDescriptorType type,
    VkBuffer buffer, uint64_t bufferId, VkDeviceSize offset, VkDeviceSize range)
{
    auto& slot = m_Sets[set].Slots[binding];
    slot.Type = type;
    slot.Buffer = buffer;
    slot.BufferId = bufferId;
    slot.BufferOffset = offset;
    slot.BufferRange = range;
    slot.Valid = true;
    m_Sets[set].Dirty = true;
}

void DescriptorState::SetImage(uint32_t set, uint32_t binding, VkDescriptorType type,
    VkImageView view, uint64_t imageViewId,
    VkSampler sampler, uint64_t samplerId, VkImageLayout imageLayout)
{
    auto& slot = m_Sets[set].Slots[binding];
    slot.Type = type;
    slot.ImageView = view;
    slot.ImageViewId = imageViewId;
    slot.Sampler = sampler;
    slot.SamplerId = samplerId;
    slot.ImageLayout = imageLayout;
    slot.Valid = true;
    m_Sets[set].Dirty = true;
}

void DescriptorState::UpdateAndBind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
    VkDevice device, DescriptorSetAllocator& allocator)
{
    for (uint32_t s = 0; s < uint32_t(m_Sets.size()); ++s)
    {
        auto& setState = m_Sets[s];
        if (!setState.Dirty) continue;

        bool cacheHit = false;
        VkDescriptorSet set = allocator.FindOrAllocate(device, setState.Layout, setState, cacheHit);

        if (!cacheHit)
        {
            m_Writer.Clear();
            for (uint32_t b = 0; b < uint32_t(setState.Slots.size()); ++b)
            {
                const auto& slot = setState.Slots[b];
                if (!slot.Valid) continue;

                const bool isBuffer =
                    slot.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                    slot.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                    slot.Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
                    slot.Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

                if (isBuffer)
                {
                    m_Writer.WriteBuffer(b, slot.Type, slot.Buffer, slot.BufferOffset, slot.BufferRange);
                } else
                {
                    m_Writer.WriteImage(b, slot.Type, slot.ImageView, slot.Sampler, slot.ImageLayout);
                }
            }
            m_Writer.UpdateSet(device, set);
        }

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, s, 1, &set, 0, nullptr);
        setState.Dirty = false;
    }
}

void DescriptorState::MarkAllDirty()
{
    for (auto& s : m_Sets)
    {
        s.Dirty = true;
    }
}

DescriptorPool* DescriptorPool::Create(VkDevice device, uint32_t maxSets,
    const std::vector<std::pair<VkDescriptorType, float>>& ratios)
{
    std::vector<VkDescriptorPoolSize> sizes;
    for (auto& [type, ratio] : ratios)
    {
        sizes.push_back({ type, std::max(1u, uint32_t(ratio * maxSets)) });
    }

    VkDescriptorPoolCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.maxSets = maxSets;
    ci.poolSizeCount = uint32_t(sizes.size());
    ci.pPoolSizes = sizes.data();

    auto* pool = new DescriptorPool();
    pool->m_Device = device;
    pool->m_MaxSets = maxSets;
    vkCreateDescriptorPool(device, &ci, nullptr, &pool->m_Pool);
    return pool;
}

DescriptorPool::~DescriptorPool()
{
    if (m_Pool != nullptr)
        vkDestroyDescriptorPool(m_Device, m_Pool, nullptr);
}

bool DescriptorPool::TryAllocate(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorSet& out)
{
    if (IsFull())
    {
        return false;
    }

    VkDescriptorSetAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    ai.descriptorPool = m_Pool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(device, &ai, &out) == VK_SUCCESS)
    {
        ++m_Used;
        return true;
    }
    return false;
}

void DescriptorPool::Reset(VkDevice device)
{
    vkResetDescriptorPool(device, m_Pool, 0);
    m_Used = 0;
}

DescriptorSetCache::~DescriptorSetCache()
{
    for (auto* pool : m_Pools)
        delete pool;
}

bool DescriptorSetCache::Find(uint64_t hash, VkDescriptorSet& out)
{
    auto it = m_Cache.find(hash);
    if (it != m_Cache.end())
    {
        out = it->second;
        return true;
    }
    return false;
}

VkDescriptorSet DescriptorSetCache::Allocate(VkDevice device, VkDescriptorSetLayout layout)
{
    DescriptorPool* pool = GetOrCreatePool(device);
    VkDescriptorSet set = VK_NULL_HANDLE;
    pool->TryAllocate(device, layout, set);
    return set;
}

void DescriptorSetCache::Register(uint64_t hash, VkDescriptorSet set)
{
    m_Cache[hash] = set;
}

void DescriptorSetCache::Reset(VkDevice device)
{
    for (auto* pool : m_Pools)
    {
        pool->Reset(device);
    }
    m_Cache.clear();
}

bool DescriptorSetCache::IsUnused(uint64_t completedTimelineValue) const
{
    return completedTimelineValue >= m_LastUsedTimelineValue;
}

DescriptorPool* DescriptorSetCache::GetOrCreatePool(VkDevice device)
{
    if (!m_Pools.empty() && !m_Pools.back()->IsFull())
    {
        return m_Pools.back();
    }

    uint32_t count = uint32_t(m_Pools.size());
    uint32_t maxSets = 32u << std::min(count, 2u);

    auto* pool = DescriptorPool::Create(device, maxSets, DefaultRatios);
    m_Pools.push_back(pool);
    return pool;
}

VkDescriptorSet DescriptorSetAllocator::FindOrAllocate(VkDevice device,
    VkDescriptorSetLayout layout, const DescriptorSet& state, bool& outCacheHit)
{
    size_t layoutKey = reinterpret_cast<size_t>(layout);
    DescriptorSetCache& cache = m_Caches[layoutKey];
    m_DirtyLayoutKeys.insert(layoutKey);

    uint64_t hash = ComputeHash(state);

    VkDescriptorSet set = VK_NULL_HANDLE;
    if (cache.Find(hash, set))
    {
        outCacheHit = true;
        return set;
    }

    outCacheHit = false;
    set = cache.Allocate(device, layout);
    cache.Register(hash, set);
    return set;
}

void DescriptorSetAllocator::CommitSubmission(uint64_t value)
{
    for (size_t key : m_DirtyLayoutKeys)
    {
        auto it = m_Caches.find(key);
        if (it != m_Caches.end())
        {
            it->second.Touch(value);
        }
    }
    m_DirtyLayoutKeys.clear();
}

void DescriptorSetAllocator::GC(uint64_t completedTimelineValue)
{
    for (auto it = m_Caches.begin(); it != m_Caches.end(); )
    {
        if (it->second.IsUnused(completedTimelineValue))
        {
            it = m_Caches.erase(it);
        } else
        {
            ++it;
        }  
    }
}

}