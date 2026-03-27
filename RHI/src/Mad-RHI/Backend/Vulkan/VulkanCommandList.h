#pragma once

#include "Mad-RHI/CommandList.h"
#include <volk/volk.h>
#include <vector>
#include "Mad-RHI/Backend/Vulkan/Vk/CommandListPool.h"

namespace mad::rhi {

class VulkanDevice;

class VulkanImmidiateCommandList : public ObjectBase<ImmidiateCommandList>
{
protected:
    ~VulkanImmidiateCommandList();

public:
    VulkanImmidiateCommandList(VulkanDevice* context);

    virtual void ResourceBarrier(
        std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers) override;

    virtual void Flush() override;

    void AddWaitSemaphore(VkSemaphore sem, uint64_t value = 0);
    void AddSignalSemaphore(VkSemaphore sem, uint64_t value = 0);

private:
    VulkanDevice* m_Context = nullptr;
    VkDevice m_Device = nullptr;

    VkQueue m_Queue = nullptr;
    uint32_t m_QueueFamilyIndex = -1;

    std::vector<VkSemaphore> m_WaitSemaphores;
    std::vector<uint64_t> m_WaitSemaphoresValues;
    std::vector<VkSemaphore> m_SignalSemaphores;
    std::vector<uint64_t> m_SignalSemaphoresValues;

    VkSemaphore m_TimelineSemaphore = nullptr;
    uint64_t m_TimelineSemaphoreValue = 0;

    vk::CommandListPool m_CommandListPool;
    VkCommandBuffer m_CurrentCommandBuffer = nullptr;

private:
    void CreateQueueSync();
    void DestroyQueueSync();

    void AcquireCommandBuffer();

};

}