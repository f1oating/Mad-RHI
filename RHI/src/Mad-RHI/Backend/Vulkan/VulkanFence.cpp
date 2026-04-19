#include "Mad-RHI/Backend/Vulkan/VulkanFence.h"
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

namespace mad::rhi {

VulkanFence::VulkanFence(VulkanDevice* context)
{
    m_Context = context;

    VkSemaphoreTypeCreateInfo stci{};
    stci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    stci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    stci.initialValue = m_TimelineSemaphoreValue;

    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    sci.pNext = &stci;

    vkCreateSemaphore(m_Context->GetDevice(), &sci, nullptr, &m_TimelineSemaphore);
}

VulkanFence::~VulkanFence()
{
    if (m_TimelineSemaphore != nullptr)
    {
        m_Context->SafeReleaseResource(new VkTimelineSemaphoreResource{ m_TimelineSemaphore, m_Context->GetDevice() });
    }
}

uint64_t VulkanFence::GetCompletedValue()
{
    uint64_t value;
    vkGetSemaphoreCounterValue(m_Context->GetDevice(), m_TimelineSemaphore,
        &value);
    return value;
}

uint64_t VulkanFence::GetCurrentValue() 
{ 
    return m_TimelineSemaphoreValue; 
}

void VulkanFence::IncrementCurrentValue() 
{ 
    m_TimelineSemaphoreValue++; 
}

void VulkanFence::Wait(uint64_t value)
{
    VkSemaphoreWaitInfo waitInfo{};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &m_TimelineSemaphore;
    waitInfo.pValues = &value;

    vkWaitSemaphores(m_Context->GetDevice(), &waitInfo, UINT64_MAX);
}

}