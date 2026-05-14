#include "Mad-RHI/Backend/Vulkan/VulkanCommandQueue.h"
#include <iostream>
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include <algorithm>
#include "Mad-RHI/Backend/Vulkan/VulkanFence.h"
#include <cstring>

namespace mad::rhi {

VulkanCommandQueue::VulkanCommandQueue(VkQueue queue, uint32_t queueFamilyIndex, VulkanDevice* context)
{
    m_Context = context;

    m_Device = m_Context->GetDevice();
    m_Queue = queue;
    m_QueueFamilyIndex = queueFamilyIndex;

    CreateQueueSync();
    m_CommandListPool.Init(m_Device, m_QueueFamilyIndex);

    AcquireCommandBuffer();

    std::cout << "VulkanImmidiateCommandList created" << std::endl;
}

VulkanCommandQueue::~VulkanCommandQueue()
{
    m_ReleaseManager.Flush();
    m_CommandListPool.Shutdown();
    DestroyQueueSync();

    std::cout << "VulkanImmidiateCommandList destroyed" << std::endl;
}

void VulkanCommandQueue::ResourceBarrier(
    std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers)
{
    EndRenderingScope();

    std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
    std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;

    for (TextureBarrier& tb : textureBarriers)
    {
        VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(tb.Texture);
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
        VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(bb.Buffer);
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

void VulkanCommandQueue::SetRenderTargets(std::vector<TextureView*> colorViews, TextureView* depthView)
{
    EndRenderingScope();

    m_ColorAttachments.clear();
    m_HasDepthAttachment = false;

    uint32_t width = 0, height = 0;

    for (TextureView* view : colorViews)
    {
        VulkanTextureView* vkView = static_cast<VulkanTextureView*>(view);

        if (width == 0)
        {
            width  = vkView->GetTexture()->GetDesc().Width;
            height = vkView->GetTexture()->GetDesc().Height;
        }

        VkRenderingAttachmentInfoKHR att{};
        att.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        att.imageView = vkView->GetView();
        att.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        att.resolveMode = VK_RESOLVE_MODE_NONE;
        att.resolveImageView = VK_NULL_HANDLE;
        att.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.clearValue = {};
        m_ColorAttachments.push_back(att);
    }

    if (depthView)
    {
        VulkanTextureView* vkView = static_cast<VulkanTextureView*>(depthView);

        m_DepthAttachment = {};
        m_DepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        m_DepthAttachment.imageView = vkView->GetView();
        m_DepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        m_DepthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
        m_DepthAttachment.resolveImageView = VK_NULL_HANDLE;
        m_DepthAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        m_DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        m_DepthAttachment.clearValue = {};
        m_HasDepthAttachment = true;
    }

    m_RenderingInfo = {};
    m_RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    m_RenderingInfo.renderArea = {{0, 0}, {width, height}};
    m_RenderingInfo.layerCount = 1;
    m_RenderingInfo.colorAttachmentCount = (uint32_t)m_ColorAttachments.size();
    m_RenderingInfo.pColorAttachments = m_ColorAttachments.data();
    m_RenderingInfo.pDepthAttachment = m_HasDepthAttachment ? &m_DepthAttachment : nullptr;
    m_RenderingInfo.pStencilAttachment = nullptr;
}

void VulkanCommandQueue::ClearRenderTarget(TextureView* view, const float color[4])
{
    VulkanTextureView* vkView = static_cast<VulkanTextureView*>(view);
    VkImageView target = vkView->GetView();

    for (VkRenderingAttachmentInfoKHR& att : m_ColorAttachments)
    {
        if (att.imageView == target)
        {
            att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            att.clearValue.color = {{ color[0], color[1], color[2], color[3] }};
            break;
        }
    }
}

void VulkanCommandQueue::ClearDepthStencil(TextureView* view, float depth, uint8_t stencil)
{
    m_DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_DepthAttachment.clearValue.depthStencil = { depth, stencil };
}

void VulkanCommandQueue::SetGraphicsPipeline(GraphicsPipelineState* pipeline)
{
    m_BoundPipeline = static_cast<VulkanGraphicsPipelineState*>(pipeline);

    std::vector<VkDescriptorSetLayout> layouts;
    for (uint32_t i = 0; i < m_BoundPipeline->GetSetCount(); ++i)
    {
        layouts.push_back(m_BoundPipeline->GetSetLayout(i));
    }

    m_DescriptorState.Init(layouts);
    m_DescriptorState.MarkAllDirty();

    vkCmdBindPipeline(m_CurrentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_BoundPipeline->GetPipeline());
}

void VulkanCommandQueue::SetVertexBuffers(uint32_t startSlot, std::vector<Buffer*> buffers, std::vector<uint64_t> offsets)
{
    std::vector<VkBuffer> vkBuffers;
    vkBuffers.reserve(buffers.size());
    for (Buffer* buf : buffers)
    {
        vkBuffers.push_back(static_cast<VulkanBuffer*>(buf)->GetBuffer());
    }
    vkCmdBindVertexBuffers(m_CurrentCommandBuffer, startSlot, (uint32_t)vkBuffers.size(), vkBuffers.data(), offsets.data());
}

void VulkanCommandQueue::SetUniformBuffer(const char* name, Buffer* buffer)
{
    auto* res = m_BoundPipeline->GetReflection().Find(name);
    if (!res) return;

    auto* vkBuf = static_cast<VulkanBuffer*>(buffer);
    m_DescriptorState.SetBuffer(res->Set, res->Binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        vkBuf->GetBuffer(), vkBuf->GetID(), vkBuf->GetOffset(), vkBuf->GetDesc().Size);
}

void VulkanCommandQueue::SetStorageBuffer(const char* name, Buffer* buffer)
{
    auto* res = m_BoundPipeline->GetReflection().Find(name);
    if (!res) return;

    auto* vkBuf = static_cast<VulkanBuffer*>(buffer);
    m_DescriptorState.SetBuffer(res->Set, res->Binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        vkBuf->GetBuffer(), vkBuf->GetID(), vkBuf->GetOffset(), vkBuf->GetDesc().Size);
}

void VulkanCommandQueue::SetSampledTexture(const char* name, TextureView* view, Sampler* sampler)
{
    auto* res = m_BoundPipeline->GetReflection().Find(name);
    if (!res) return;

    auto* vkView = static_cast<VulkanTextureView*>(view);
    auto* vkSampler = static_cast<VulkanSampler*>(sampler);
    m_DescriptorState.SetImage(res->Set, res->Binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        vkView->GetView(), vkView->GetID(),
        vkSampler->GetSampler(), vkSampler->GetID(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VulkanCommandQueue::SetTexture(const char* name, TextureView* view)
{
    auto* res = m_BoundPipeline->GetReflection().Find(name);
    if (!res) return;

    auto* vkView = static_cast<VulkanTextureView*>(view);
    m_DescriptorState.SetImage(res->Set, res->Binding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        vkView->GetView(), vkView->GetID(), nullptr, 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VulkanCommandQueue::SetSampler(const char* name, Sampler* sampler)
{
    auto* res = m_BoundPipeline->GetReflection().Find(name);
    if (!res) return;

    auto* vkSampler = static_cast<VulkanSampler*>(sampler);
    m_DescriptorState.SetImage(res->Set, res->Binding, VK_DESCRIPTOR_TYPE_SAMPLER,
        nullptr, 0, vkSampler->GetSampler(), vkSampler->GetID(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VulkanCommandQueue::Draw(uint32_t numVertices, uint32_t firstVertex)
{
    BeginRenderingIfNeeded();

    if (m_DescriptorState.HasAnySets())
    {
        m_DescriptorState.UpdateAndBind(
            m_CurrentCommandBuffer,
            m_BoundPipeline->GetPipelineLayout(),
            m_Device,
            m_DescriptorAllocator);
    }

    vkCmdDraw(m_CurrentCommandBuffer, numVertices, 1, firstVertex, 0);
}

void VulkanCommandQueue::UpdateTexture(Texture* texture, const void* data, uint64_t size)
{
    VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(texture);
    const TextureDesc& desc = vulkanTexture->GetDesc();
    VmaAllocator allocator = m_Context->GetVmaAllocator();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VkBufferCreateInfo bci{};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = size;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    vmaCreateBuffer(allocator, &bci, &aci, &stagingBuffer, &stagingAllocation, nullptr);

    void* mapped;
    vmaMapMemory(allocator, stagingAllocation, &mapped);
    memcpy(mapped, data, size);
    vmaUnmapMemory(allocator, stagingAllocation);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { desc.Width, desc.Height, 1 };

    vkCmdCopyBufferToImage(
        m_CurrentCommandBuffer,
        stagingBuffer,
        vulkanTexture->GetImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);

    SafeReleaseResource(vk::StaleResourceWrapper::Create(new VkBufferResource{ stagingBuffer, stagingAllocation, allocator }));
}

void VulkanCommandQueue::UpdateBuffer(Buffer* buffer, const void* data, uint64_t size)
{
    VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);
    VmaAllocator allocator = m_Context->GetVmaAllocator();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VkBufferCreateInfo bci{};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = size;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    vmaCreateBuffer(allocator, &bci, &aci, &stagingBuffer, &stagingAllocation, nullptr);

    void* mapped;
    vmaMapMemory(allocator, stagingAllocation, &mapped);
    memcpy(mapped, data, size);
    vmaUnmapMemory(allocator, stagingAllocation);

    VkBufferCopy region{};
    region.size = size;
    vkCmdCopyBuffer(m_CurrentCommandBuffer, stagingBuffer, vulkanBuffer->GetBuffer(), 1, &region);

    SafeReleaseResource(vk::StaleResourceWrapper::Create(new VkBufferResource{ stagingBuffer, stagingAllocation, allocator }));
}

void VulkanCommandQueue::EnqueueSignal(Fence* fence, uint64_t value)
{
    VulkanFence* vulkanFence = static_cast<VulkanFence*>(fence);

    AddSignalSemaphore(vulkanFence->GetTimelineSemaphore(), value);
}

void VulkanCommandQueue::WaitForFence(Fence* fence, uint64_t value)
{
    VulkanFence* vulkanFence = static_cast<VulkanFence*>(fence);

    AddWaitSemaphore(vulkanFence->GetTimelineSemaphore(), value);
}

void VulkanCommandQueue::Flush()
{
    EndRenderingScope();
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

    m_DescriptorAllocator.CommitSubmission(m_TimelineSemaphoreValue);
    m_CommandListPool.ReleaseCommandBuffer(m_CurrentCommandBuffer, m_TimelineSemaphoreValue);
    m_ReleaseManager.DiscardStaleResources(m_CommandBufferNumber, m_TimelineSemaphoreValue);

    AcquireCommandBuffer();

    m_CommandListPool.Purge(GetTimelineSemaphoreValue());
}

void VulkanCommandQueue::SafeReleaseResource(vk::StaleResourceWrapper&& wrapper)
{
    m_ReleaseManager.SafeReleaseResource(std::move(wrapper), m_CommandBufferNumber);
}

void VulkanCommandQueue::SafeReleaseResource(const vk::StaleResourceWrapper& wrapper)
{
    m_ReleaseManager.SafeReleaseResource(wrapper, m_CommandBufferNumber);
}

void VulkanCommandQueue::EndFrame()
{
    Flush();
    uint64_t value = GetTimelineSemaphoreValue();
    m_ReleaseManager.Purge(value);
    m_DescriptorAllocator.GC(m_Device, value);
}

void VulkanCommandQueue::FlushWaitSemaphores()
{
    m_WaitSemaphores.clear();
    m_WaitSemaphoresValues.clear();
}

void VulkanCommandQueue::AddWaitSemaphore(VkSemaphore sem, uint64_t value)
{
    if (std::find(m_WaitSemaphores.begin(), m_WaitSemaphores.end(), sem) != m_WaitSemaphores.end())
        return;

    m_WaitSemaphores.push_back(sem);
    m_WaitSemaphoresValues.push_back(value);
}

void VulkanCommandQueue::FlushSignalSemaphores()
{
    m_SignalSemaphores.clear();
    m_SignalSemaphoresValues.clear();
}

void VulkanCommandQueue::AddSignalSemaphore(VkSemaphore sem, uint64_t value)
{
    if (std::find(m_SignalSemaphores.begin(), m_SignalSemaphores.end(), sem) != m_SignalSemaphores.end())
        return;

    m_SignalSemaphores.push_back(sem);
    m_SignalSemaphoresValues.push_back(value);
}

void VulkanCommandQueue::CreateQueueSync()
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

void VulkanCommandQueue::DestroyQueueSync()
{
    if (m_TimelineSemaphore != nullptr)
    {
        vkDestroySemaphore(m_Device, m_TimelineSemaphore, nullptr);
    }
}

uint64_t VulkanCommandQueue::GetTimelineSemaphoreValue()
{
    uint64_t value;
    vkGetSemaphoreCounterValue(m_Device, m_TimelineSemaphore,
        &value);
    return value;
}

void VulkanCommandQueue::AcquireCommandBuffer()
{
    m_CurrentCommandBuffer = m_CommandListPool.AcquireCommandBuffer();
    VkCommandBufferBeginInfo cbbi{};
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(m_CurrentCommandBuffer, &cbbi);

    m_CommandBufferNumber++;
}

void VulkanCommandQueue::BeginRenderingIfNeeded()
{
    if (m_IsInRenderingScope) return;
    if (m_ColorAttachments.empty() && !m_HasDepthAttachment) return;

    m_RenderingInfo.pColorAttachments = m_ColorAttachments.data();
    m_RenderingInfo.pDepthAttachment = m_HasDepthAttachment ? &m_DepthAttachment : nullptr;

    vkCmdBeginRendering(m_CurrentCommandBuffer, &m_RenderingInfo);
    
    VkViewport viewport{};
    viewport.width    = (float)m_RenderingInfo.renderArea.extent.width;
    viewport.height   = (float)m_RenderingInfo.renderArea.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_CurrentCommandBuffer, 0, 1, &viewport);

    VkRect2D scissor{ {0, 0}, m_RenderingInfo.renderArea.extent };
    vkCmdSetScissor(m_CurrentCommandBuffer, 0, 1, &scissor);

    m_IsInRenderingScope = true;
}

void VulkanCommandQueue::EndRenderingScope()
{
    if (!m_IsInRenderingScope) return;
    vkCmdEndRendering(m_CurrentCommandBuffer);
    m_IsInRenderingScope = false;
}

}