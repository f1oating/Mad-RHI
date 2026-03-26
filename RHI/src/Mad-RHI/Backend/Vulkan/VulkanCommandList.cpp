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
    m_CommandListPool.Init(m_Device, m_QueueFamilyIndex);

    AcquireCommandBuffer();

    std::cout << "VulkanImmidiateCommandList created" << std::endl;
}

VulkanImmidiateCommandList::~VulkanImmidiateCommandList()
{
    m_CommandListPool.Shutdown();
    DestroyQueueSync();

    std::cout << "VulkanImmidiateCommandList destroyed" << std::endl;
}

void VulkanImmidiateCommandList::Flush()
{
    vkEndCommandBuffer(m_CurrentCommandBuffer);

    AddSignalSemaphore(m_TimelineSemaphore, ++m_TimelineSemaphoreValue);

    std::vector<VkPipelineStageFlags> waitStages(m_WaitSemaphores.size(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VkTimelineSemaphoreSubmitInfo tssi{};
    tssi.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    tssi.waitSemaphoreValueCount = (uint32_t)m_WaitSemaphoresValues.size();
    tssi.pWaitSemaphoreValues = m_WaitSemaphoresValues.data();
    tssi.signalSemaphoreValueCount = (uint32_t)m_SignalSemaphoresValues.size();
    tssi.pSignalSemaphoreValues = m_SignalSemaphoresValues.data();

    VkSubmitInfo si{};
    si.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.pNext                = &tssi;
    si.waitSemaphoreCount   = (uint32_t)m_WaitSemaphores.size();
    si.pWaitSemaphores      = m_WaitSemaphores.data();
    si.pWaitDstStageMask    = waitStages.data();
    si.commandBufferCount   = 1;
    si.pCommandBuffers      = &m_CurrentCommandBuffer;
    si.signalSemaphoreCount = (uint32_t)m_SignalSemaphores.size();
    si.pSignalSemaphores    = m_SignalSemaphores.data();

    vkQueueSubmit(m_Queue, 1, &si, VK_NULL_HANDLE);

    m_WaitSemaphores.clear();
    m_WaitSemaphoresValues.clear();
    m_SignalSemaphores.clear();
    m_SignalSemaphoresValues.clear();

    m_CommandListPool.ReleaseCommandBuffer(m_CurrentCommandBuffer, m_TimelineSemaphoreValue);
    AcquireCommandBuffer();

    uint64_t value;
    vkGetSemaphoreCounterValue(m_Device, m_TimelineSemaphore,
        &value);

    m_CommandListPool.Purge(value);
    m_Context->GetReleaseManager()->Purge(value);
}

void VulkanImmidiateCommandList::AddWaitSemaphore(VkSemaphore sem, uint64_t value)
{
    m_WaitSemaphores.push_back(sem);
    m_WaitSemaphoresValues.push_back(value);
}

void VulkanImmidiateCommandList::AddSignalSemaphore(VkSemaphore sem, uint64_t value)
{
    m_SignalSemaphores.push_back(sem);
    m_SignalSemaphoresValues.push_back(value);
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

void VulkanImmidiateCommandList::AcquireCommandBuffer()
{
    m_CurrentCommandBuffer = m_CommandListPool.AcquireCommandBuffer();
    VkCommandBufferBeginInfo cbbi{};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_CurrentCommandBuffer, &cbbi);
}

}