#include "Mad-RHI/Backend/Vulkan/VulkanCommandList.h"
#include <iostream>
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

namespace mad::rhi {

VulkanImmidiateCommandList::VulkanImmidiateCommandList(VulkanDevice* vulkanDevice)
{
    m_VulkanDevice = vulkanDevice;

    m_Queue = m_VulkanDevice->GetGrpaphicsQueue();
    m_QueueFamilyIndex = m_VulkanDevice->GetGrpaphicsQueueFamilyIndex();

    std::cout << "VulkanImmidiateCommandList created" << std::endl;
}

VulkanImmidiateCommandList::~VulkanImmidiateCommandList()
{
    std::cout << "VulkanImmidiateCommandList destroyed" << std::endl;
}

void VulkanImmidiateCommandList::AddWaitSemaphore(VkSemaphore sem)
{
    m_WaitSemaphores.push_back(sem);
}

void VulkanImmidiateCommandList::AddSignalSemaphore(VkSemaphore sem)
{
    m_SignalSemaphores.push_back(sem);
}

}