#pragma once

#include <cstdint>
#include <volk/volk.h>
#include <deque>

namespace mad::rhi::vk {

class CommandListPool
{
struct Entry
{
    VkCommandBuffer CommandBuffer;
    uint64_t FenceValue;
};

public:
    CommandListPool() = default;
    ~CommandListPool() = default;

    void Init(VkDevice device, uint32_t queueIndex);
    void Shutdown();

    VkCommandBuffer AcquireCommandBuffer();
    void ReleaseCommandBuffer(VkCommandBuffer cb, uint64_t fenceValue);

    void Purge(uint64_t fenceValue);

private:
    VkDevice m_Device = nullptr;
    VkCommandPool m_CommandPool = nullptr;

    std::deque<VkCommandBuffer> m_AcquireQueue;
    std::deque<Entry> m_ReleaseQueue;

};

}