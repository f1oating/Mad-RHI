#include "Mad-RHI/Backend/Vulkan/VulkanCommandList.h"
#include <iostream>
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include <algorithm>

namespace mad::rhi {

VulkanImmidiateCommandList::VulkanImmidiateCommandList(VulkanDevice* context)
{
    m_Context = context;

    m_Device = m_Context->GetDevice();
    m_Queue = m_Context->GetGraphicsQueue();
    m_QueueFamilyIndex = m_Context->GetGraphicsQueueFamilyIndex();

    CreateQueueSync();
    m_CommandListPool.Init(m_Device, m_QueueFamilyIndex);
    m_ReleaseManager.Init(m_Device);

    AcquireCommandBuffer();

    std::cout << "VulkanImmidiateCommandList created" << std::endl;
}

VulkanImmidiateCommandList::~VulkanImmidiateCommandList()
{
    m_ReleaseManager.Shutdown();
    m_CommandListPool.Shutdown();
    DestroyQueueSync();

    std::cout << "VulkanImmidiateCommandList destroyed" << std::endl;
}

void VulkanImmidiateCommandList::ResourceBarrier(
    std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers)
{
    std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
    std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;

    for (TextureBarrier& tb : textureBarriers)
    {
        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(tb.Texture.Get());
        VkImageMemoryBarrier imgBarrier{};
        imgBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask       = ToVkAccessMask(vulkanTexture->GetCurrentResourceState());
        imgBarrier.dstAccessMask       = ToVkAccessMask(tb.NewState);
        imgBarrier.oldLayout           = ToVkImageLayout(vulkanTexture->GetCurrentResourceState());
        imgBarrier.newLayout           = ToVkImageLayout(tb.NewState);
        imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imgBarrier.image               = vulkanTexture->GetImage();
        imgBarrier.subresourceRange    = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = tb.BaseMip,
            .levelCount     = tb.MipCount ? tb.MipCount : VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = tb.BaseSlice,
            .layerCount     = tb.SliceCount ? tb.SliceCount : VK_REMAINING_ARRAY_LAYERS,
        };
        imageMemoryBarriers.emplace_back(imgBarrier);
        vulkanTexture->SetResourceState(tb.NewState);
    }

    for (BufferBarrier& bb : bufferBarriers)
    {
        VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(bb.Buffer.Get());
        VkBufferMemoryBarrier bufBarrier{};
        bufBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        bufBarrier.srcAccessMask       = ToVkAccessMask(vulkanBuffer->GetCurrentResourceState());
        bufBarrier.dstAccessMask       = ToVkAccessMask(bb.NewState);
        bufBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        bufBarrier.buffer              = vulkanBuffer->GetBuffer();
        bufBarrier.offset              = 0;
        bufBarrier.size                = VK_WHOLE_SIZE;
        vulkanBuffer->SetResourceState(bb.NewState);
    }

    vkCmdPipelineBarrier(
        m_CurrentCommandBuffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0, nullptr,
        bufferMemoryBarriers.size(),
        bufferMemoryBarriers.data(),
        imageMemoryBarriers.size(),
        imageMemoryBarriers.data()
    );
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
    m_ReleaseManager.DiscardStaleResources(m_CommandBufferNumber, m_TimelineSemaphoreValue);

    AcquireCommandBuffer();

    m_CommandListPool.Purge(GetTimelineSemaphoreValue());
}

void VulkanImmidiateCommandList::SafeReleaseResource(vk::StaleResourceWrapper&& wrapper)
{
    m_ReleaseManager.SafeReleaseResource(wrapper, m_CommandBufferNumber);
}

void VulkanImmidiateCommandList::SafeReleaseResource(const vk::StaleResourceWrapper& wrapper)
{
    m_ReleaseManager.SafeReleaseResource(wrapper, m_CommandBufferNumber);
}

void VulkanImmidiateCommandList::EndFrame()
{
    Flush();
    m_ReleaseManager.Purge(GetTimelineSemaphoreValue());
}

void VulkanImmidiateCommandList::FlushWaitSemaphores()
{
    m_WaitSemaphores.clear();
    m_WaitSemaphoresValues.clear();
}

void VulkanImmidiateCommandList::AddWaitSemaphore(VkSemaphore sem, uint64_t value)
{
    if (std::find(m_WaitSemaphores.begin(), m_WaitSemaphores.end(), sem) != m_WaitSemaphores.end())
        return;

    m_WaitSemaphores.push_back(sem);
    m_WaitSemaphoresValues.push_back(value);
}

void VulkanImmidiateCommandList::FlushSignalSemaphores()
{
    m_SignalSemaphores.clear();
    m_SignalSemaphoresValues.clear();
}

void VulkanImmidiateCommandList::AddSignalSemaphore(VkSemaphore sem, uint64_t value)
{
    if (std::find(m_SignalSemaphores.begin(), m_SignalSemaphores.end(), sem) != m_SignalSemaphores.end())
        return;

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

uint64_t VulkanImmidiateCommandList::GetTimelineSemaphoreValue()
{
    uint64_t value;
    vkGetSemaphoreCounterValue(m_Device, m_TimelineSemaphore,
        &value);
    return value;
}

void VulkanImmidiateCommandList::AcquireCommandBuffer()
{
    m_CurrentCommandBuffer = m_CommandListPool.AcquireCommandBuffer();
    VkCommandBufferBeginInfo cbbi{};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_CurrentCommandBuffer, &cbbi);

    m_CommandBufferNumber++;
}

}