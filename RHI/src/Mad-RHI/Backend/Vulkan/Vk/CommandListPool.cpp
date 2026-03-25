#include "Mad-RHI/Backend/Vulkan/Vk/CommandListPool.h"

namespace mad::rhi::vk {

void CommandListPool::Init(VkDevice device, uint32_t queueIndex)
{
    m_Device = device;

    VkCommandPoolCreateInfo cpci{};
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.queueFamilyIndex = queueIndex;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(device, &cpci, nullptr, &m_CommandPool);
}

void CommandListPool::Shutdown()
{
    if (m_CommandPool != nullptr)
    {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    }
}

VkCommandBuffer CommandListPool::AcquireCommandBuffer()
{
    if (!m_AcquireQueue.empty())
    {
        VkCommandBuffer cb = m_AcquireQueue.front();
        m_AcquireQueue.pop_front();
        vkResetCommandBuffer(cb, 0);
        return cb;
    }

    VkCommandBufferAllocateInfo cbai{};
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool = m_CommandPool;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer cb = nullptr;
    vkAllocateCommandBuffers(m_Device, &cbai, &cb);
    
    return cb;
}

void CommandListPool::ReleaseCommandBuffer(VkCommandBuffer cb, uint64_t fenceValue)
{
    m_ReleaseQueue.push_back({ cb, fenceValue });
}

void CommandListPool::Purge(uint64_t fenceValue)
{
    while (!m_ReleaseQueue.empty())
    {
        auto& entry = m_ReleaseQueue.front();
        if (entry.FenceValue > fenceValue) break;
        m_AcquireQueue.push_back(entry.CommandBuffer);
        m_ReleaseQueue.pop_front();
    }
}
    
}