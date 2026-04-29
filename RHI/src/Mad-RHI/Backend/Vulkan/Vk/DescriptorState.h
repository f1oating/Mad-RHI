#pragma once

#include <volk/volk.h>
#include <deque>
#include <vector>

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
    VkBuffer Buffer = nullptr;
    VkDeviceSize BufferOffset = 0;
    VkDeviceSize BufferRange = 0;
    VkImageView ImageView = nullptr;
    VkSampler Sampler = nullptr;
    VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct DescriptorSetState
{
    VkDescriptorSetLayout Layout = nullptr;
    std::vector<DescriptorSlot> Slots;
    bool Dirty = false;
};

class DescriptorState
{
public:
    void Init(const std::vector<VkDescriptorSetLayout>& setLayouts);

    void SetBuffer(uint32_t set, uint32_t binding, VkDescriptorType type,
        VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

    void SetImage(uint32_t set, uint32_t binding, VkDescriptorType type,
        VkImageView view, VkSampler sampler, VkImageLayout imageLayout);

    void UpdateAndBind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
        VkDevice device);

    void MarkAllDirty();
    bool HasAnySets() const { return !m_Sets.empty(); }

private:
    std::vector<DescriptorSetState> m_Sets;
    DescriptorWriter m_Writer;

};

}