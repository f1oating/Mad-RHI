#pragma once

#include "Mad-RHI/CommandQueue.h"
#include <volk/volk.h>
#include <vector>
#include "Mad-RHI/Backend/Vulkan/Vk/CommandListPool.h"
#include "Mad-RHI/Backend/Vulkan/Vk/ReleaseManager.h"
#include "Mad-RHI/Backend/Vulkan/Vk/DescriptorState.h"
#include "Mad-RHI/Backend/Vulkan/VulkanPipelineState.h"

namespace mad::rhi {

class VulkanDevice;

class VulkanCommandQueue : public ObjectBase<CommandQueue>
{
protected:
    ~VulkanCommandQueue();

public:
    VulkanCommandQueue(VkQueue queue, uint32_t queueFamilyIndex, VulkanDevice* context);

    virtual void ResourceBarrier(
        std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers) override;

    virtual void SetRenderTargets(std::vector<TextureView*> colorViews, TextureView* depthView) override;
    virtual void ClearRenderTarget(TextureView* view, const float color[4]) override;
    virtual void ClearDepthStencil(TextureView* view, float depth, uint8_t stencil) override;

    virtual void SetGraphicsPipeline(GraphicsPipelineState* pipeline) override;

    virtual void SetVertexBuffers(uint32_t startSlot, std::vector<Buffer*> buffers, std::vector<uint64_t> offsets) override;

    virtual void SetUniformBuffer(const char* name, Buffer* buffer) override;
    virtual void SetStorageBuffer(const char* name, Buffer* buffer) override;
    virtual void SetSampledTexture(const char* name, TextureView* view, Sampler* sampler) override;
    virtual void SetTexture(const char* name, TextureView* view) override;
    virtual void SetSampler(const char* name, Sampler* sampler) override;

    virtual void Draw(uint32_t numVertices, uint32_t firstVertex) override;

    virtual void UpdateTexture(Texture* texture, const void* data, uint64_t size) override;
    virtual void UpdateBuffer(Buffer* buffer, const void* data, uint64_t size) override;
    
    virtual void EnqueueSignal(Fence* fence, uint64_t value) override;
    virtual void WaitForFence(Fence* fence, uint64_t value) override;

    virtual void Flush() override;

    void SafeReleaseResource(vk::StaleResourceWrapper&& wrapper);
    void SafeReleaseResource(const vk::StaleResourceWrapper& wrapper);

    void EndFrame();

    void FlushWaitSemaphores();
    void AddWaitSemaphore(VkSemaphore sem, uint64_t value = 0);
    void FlushSignalSemaphores();
    void AddSignalSemaphore(VkSemaphore sem, uint64_t value = 0);

    VkQueue GetQueue() { return m_Queue; }

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

    VulkanGraphicsPipelineState* m_BoundPipeline = nullptr;

    std::vector<VkRenderingAttachmentInfoKHR> m_ColorAttachments;
    VkRenderingAttachmentInfoKHR m_DepthAttachment{};
    VkRenderingInfoKHR m_RenderingInfo{};
    bool m_HasDepthAttachment = false;
    bool m_IsInRenderingScope = false;

    vk::DescriptorState m_DescriptorState;
    vk::DescriptorSetAllocator m_DescriptorAllocator;

    bool m_HasRecordedCommands = false;

private:
    void CreateQueueSync();
    void DestroyQueueSync();

    uint64_t GetTimelineSemaphoreValue();
    void AcquireCommandBuffer();

    void BeginRenderingIfNeeded();
    void EndRenderingScope();

};

}