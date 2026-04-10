#pragma once

#include "Mad-RHI/Device.h"
#include "Mad-RHI/Factory.h"
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include <volk/volk.h>
#include <vector>
#include "Mad-RHI/Backend/Vulkan/VulkanCommandList.h"
#include <vk_mem_alloc.h>
#include "Mad-RHI/Backend/Vulkan/Vk/RingAllocator.h"

namespace mad::rhi {

class VulkanDevice : public ObjectBase<Device>
{
protected:
    ~VulkanDevice();

public:
    VulkanDevice(VkInstance instance, const WindowHandle& wh);

    virtual void Resize() override;

    virtual void EndFrame() override;
    virtual void GarbageCollect() override;

    virtual void Present() override;

    virtual RefPtr<ImmidiateCommandList> GetImmidiateCommandList() override;

    virtual RefPtr<Texture> CreateTexture(const TextureDesc& desc) override;
    virtual RefPtr<Buffer> CreateBuffer(const BufferDesc& desc) override;
    virtual RefPtr<Shader> CreateShader(const uint32_t* data, uint64_t size) override;
    virtual RefPtr<GraphicsPipelineState> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;

    void SafeReleaseResource(vk::StaleResourceBase* resource);

    VkDevice GetDevice() { return m_Device; }
    VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
    uint32_t GetGraphicsQueueFamilyIndex() { return m_GraphicsFamily; }
    vk::RingBuffer* GetRingBuffer() { return &m_RingBuffer; }

private:
    VkInstance m_Instance = nullptr;
    VkSurfaceKHR m_Surface = nullptr;
    VkPhysicalDevice m_PhysicalDevice = nullptr;
    VkDevice m_Device = nullptr;
    
    VkSwapchainKHR m_Swapchain = nullptr;
    std::vector<RefPtr<VulkanTexture>> m_SwapchainImages;

    VkQueue m_GraphicsQueue = nullptr;
    VkQueue m_PresentQueue = nullptr;
    uint32_t m_GraphicsFamily = -1;
    uint32_t m_PresentFamily = -1;

    std::vector<VkSemaphore> m_RenderFinishedSamephores;
    std::vector<VkSemaphore> m_PresentCompleteSemaphores;
    std::vector<VkFence> m_Fences;

    uint32_t m_CurrentFrame = 0;
    uint32_t m_CurrentImageIndex = 0;

    RefPtr<VulkanImmidiateCommandList> m_GraphicsImmidiateCommandList = nullptr;

    VmaAllocator m_Allocator = nullptr;

    vk::RingBuffer m_RingBuffer;

private:
    void CreateSurface(const WindowHandle& wh);
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void DestroySwapchain();
    void CreateFramesInFlightSync();
    void DestroyFramesInFlightSync();
    void CreateAllocator();

    void AcquireNextImage();

};

}