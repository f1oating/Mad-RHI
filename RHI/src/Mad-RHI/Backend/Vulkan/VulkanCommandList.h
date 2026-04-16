#pragma once

#include "Mad-RHI/CommandList.h"
#include <volk/volk.h>
#include <vector>
#include "Mad-RHI/Backend/Vulkan/Vk/CommandListPool.h"
#include "Mad-RHI/Backend/Vulkan/Vk/ReleaseManager.h"

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

    virtual void SetRenderTargets(std::vector<TextureView*> colorViews, TextureView* depthView) override;
    virtual void ClearRenderTarget(TextureView* view, const float color[4]) override;
    virtual void ClearDepthStencil(TextureView* view, float depth, uint8_t stencil) override;

    virtual void Draw(uint32_t numVertices, uint32_t firstVertex) override;

    virtual void Flush() override;

    void SafeReleaseResource(vk::StaleResourceWrapper&& wrapper);
    void SafeReleaseResource(const vk::StaleResourceWrapper& wrapper);

    void EndFrame();
    void GarbageCollect();

    void FlushWaitSemaphores();
    void AddWaitSemaphore(VkSemaphore sem, uint64_t value = 0);
    void FlushSignalSemaphores();
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
    uint64_t m_CommandBufferNumber = 0;
    VkCommandBuffer m_CurrentCommandBuffer = nullptr;

    vk::ReleaseManager m_ReleaseManager;

    std::vector<VkRenderingAttachmentInfoKHR> m_ColorAttachments;
    VkRenderingAttachmentInfoKHR m_DepthAttachment{};
    VkRenderingInfoKHR m_RenderingInfo{};
    bool m_HasDepthAttachment = false;
    bool m_IsInRenderingScope = false;

private:
    void CreateQueueSync();
    void DestroyQueueSync();

    uint64_t GetTimelineSemaphoreValue();
    void AcquireCommandBuffer();

    void BeginRenderingIfNeeded();
    void EndRenderingScope();

};

}