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
    VulkanImmidiateCommandList(VulkanDevice* vulkanDevice);

    void AddWaitSemaphore(VkSemaphore sem);
    void AddSignalSemaphore(VkSemaphore sem);

private:
    VulkanDevice* m_VulkanDevice = nullptr;

    VkQueue m_Queue = nullptr;
    uint32_t m_QueueFamilyIndex = -1;

    std::vector<VkSemaphore> m_WaitSemaphores;
    std::vector<VkSemaphore> m_SignalSemaphores;

};

}