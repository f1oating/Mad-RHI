#pragma once

#include <volk/volk.h>

namespace mad::rhi::vk {

class DescriptorSetAllocator
{
public:
    void Init(VkDevice device, uint32_t maxSets = 1024);
    void Shutdown();

    VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
    void Reset();

private:
    void CreatePool();

private:
    VkDevice m_Device = nullptr;
    uint32_t m_MaxSets = 0;
    
    VkDescriptorPool m_Pool = nullptr;

};

}