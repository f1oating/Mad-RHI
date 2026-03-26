#pragma once

#include "Mad-RHI/CommandList.h"
#include <volk/volk.h>
#include <vector>

namespace mad::rhi {

class VulkanDevice;

class VulkanImmidiateCommandList : public ObjectBase<ImmidiateCommandList>
{
protected:
    ~VulkanImmidiateCommandList();

public:
    VulkanImmidiateCommandList(VulkanDevice* context);

    virtual void Flush() override;

    uint64_t GetQueueTimelineSemaphoreGPUValue();

    void AddWaitSemaphore(VkSemaphore sem);
    void AddSignalSemaphore(VkSemaphore sem);

private:
    VulkanDevice* m_Context = nullptr;
    VkDevice m_Device = nullptr;

    VkQueue m_Queue = nullptr;
    uint32_t m_QueueFamilyIndex = -1;

    std::vector<VkSemaphore> m_WaitSemaphores;
    std::vector<VkSemaphore> m_SignalSemaphores;

    VkSemaphore m_TimelineSemaphore = nullptr;
    uint64_t m_TimelineSemaphoreValue = 0;

private:
    void CreateQueueSync();
    void DestroyQueueSync();

};

}