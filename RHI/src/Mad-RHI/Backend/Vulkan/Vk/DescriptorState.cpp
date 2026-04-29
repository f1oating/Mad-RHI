#include "Mad-RHI/Backend/Vulkan/Vk/DescriptorState.h"
#include <unordered_map>

namespace mad::rhi::vk {

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
        m_Sets[i].Dirty  = false;
    }
}

void DescriptorState::SetBuffer(uint32_t set, uint32_t binding, VkDescriptorType type,
    VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
{
    auto& slot = m_Sets[set].Slots[binding];
    slot.Type = type;
    slot.Buffer = buffer;
    slot.BufferOffset = offset;
    slot.BufferRange = range;
    slot.Valid = true;

    m_Sets[set].Dirty = true;
}

void DescriptorState::SetImage(uint32_t set, uint32_t binding, VkDescriptorType type,
    VkImageView view, VkSampler sampler, VkImageLayout imageLayout)
{
    auto& slot = m_Sets[set].Slots[binding];
    slot.Type = type;
    slot.ImageView = view;
    slot.Sampler = sampler;
    slot.ImageLayout = imageLayout;
    slot.Valid = true;

    m_Sets[set].Dirty = true;
}

void DescriptorState::UpdateAndBind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
    VkDevice device)
{
    for (uint32_t s = 0; s < (uint32_t)m_Sets.size(); ++s)
    {
        auto& setState = m_Sets[s];
        if (!setState.Dirty)
            continue;

        VkDescriptorSet set = nullptr; // cache.Find(hash);
        if (set == VK_NULL_HANDLE)
        {
            set = nullptr; // cache.Allocate(setState.Layout, GetPoolSizes(setState));

            m_Writer.Clear();
            for (uint32_t b = 0; b < (uint32_t)setState.Slots.size(); ++b)
            {
                const auto& slot = setState.Slots[b];
                if (!slot.Valid)
                    continue;

                const bool isBuffer =
                    slot.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                    slot.Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                    slot.Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
                    slot.Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

                if (isBuffer)
                {
                    m_Writer.WriteBuffer(b, slot.Type,
                        slot.Buffer, slot.BufferOffset, slot.BufferRange);
                } else
                {
                    m_Writer.WriteImage(b, slot.Type,
                        slot.ImageView, slot.Sampler, slot.ImageLayout);
                }
            }

            m_Writer.UpdateSet(device, set);
            // cache.Register(hash, set);
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

}