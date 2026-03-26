#include "Mad-RHI/Backend/Vulkan/VulkanCommandList.h"
#include <iostream>
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

namespace mad::rhi {

VulkanImmidiateCommandList::VulkanImmidiateCommandList(VulkanDevice* context)
{
    m_Context = context;

    m_Device = m_Context->GetDevice();
    m_Queue = m_Context->GetGraphicsQueue();
    m_QueueFamilyIndex = m_Context->GetGraphicsQueueFamilyIndex();

    CreateQueueSync();

    std::cout << "VulkanImmidiateCommandList created" << std::endl;
}

VulkanImmidiateCommandList::~VulkanImmidiateCommandList()
{
    DestroyQueueSync();

    std::cout << "VulkanImmidiateCommandList destroyed" << std::endl;
}

void VulkanImmidiateCommandList::Flush()
{

}

uint64_t VulkanImmidiateCommandList::GetQueueTimelineSemaphoreGPUValue()
{
    uint64_t value;
    vkGetSemaphoreCounterValue(m_Device, m_TimelineSemaphore,
        &value);
    return value;
}

void VulkanImmidiateCommandList::AddWaitSemaphore(VkSemaphore sem)
{
    m_WaitSemaphores.push_back(sem);
}

void VulkanImmidiateCommandList::AddSignalSemaphore(VkSemaphore sem)
{
    m_SignalSemaphores.push_back(sem);
}

void VulkanImmidiateCommandList::CreateQueueSync()
{
    VkSemaphoreTypeCreateInfo stci{};
    stci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    stci.initialValue = m_TimelineSemaphoreValue;

    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    sci.pNext = &stci;

    vkCreateSemaphore(m_Device, &sci, nullptr, &m_TimelineSemaphore);
}

void VulkanImmidiateCommandList::DestroyQueueSync()
{
    if (m_TimelineSemaphore != nullptr)
    {
        vkDestroySemaphore(m_Device, m_TimelineSemaphore, nullptr);
    }
}

}