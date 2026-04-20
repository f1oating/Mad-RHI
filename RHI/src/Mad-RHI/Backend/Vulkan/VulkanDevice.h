#pragma once

#include "Mad-RHI/Device.h"
#include "Mad-RHI/Factory.h"
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include <volk/volk.h>
#include <vector>
#include "Mad-RHI/Backend/Vulkan/VulkanCommandList.h"
#include <vk_mem_alloc.h>
#include "Mad-RHI/Backend/Vulkan/Vk/RingAllocator.h"
#include "Mad-RHI/Backend/Vulkan/VulkanFactory.h"

namespace mad::rhi {

class VulkanDevice : public ObjectBase<Device>
{
protected:
    ~VulkanDevice();

public:
    VulkanDevice(const DeviceDesc& desc, VulkanFactory* factory);

    virtual void EndFrame() override;
    virtual void GarbageCollect() override;

    virtual Texture* GetCurrentBackBuffer() override;

    virtual void Present() override;

    virtual RefPtr<ImmidiateCommandList> GetImmidiateCommandList() override;

    virtual void CreateTexture(Texture** ppTex, const TextureDesc& desc) override;
    virtual void CreateBuffer(Buffer** ppBuff, const BufferDesc& desc) override;
    virtual void CreateSampler(Sampler** ppSampler, const SamplerDesc& desc) override;
    virtual void CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size) override;
    virtual void CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc) override;
    virtual void CreateFence(Fence** ppFence) override;

    void SafeReleaseResource(vk::StaleResourceBase* resource);

    VkDevice GetDevice() { return m_Device; }
    VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
    uint32_t GetGraphicsQueueFamilyIndex() { return m_GraphicsFamily; }
    VmaAllocator GetVmaAllocator() { return m_Allocator; }
    vk::RingBuffer* GetRingBuffer() { return &m_RingBuffer; }

private:
    VulkanFactory* m_Factory = nullptr;

    VkInstance m_Instance = nullptr;
    VkSurfaceKHR m_Surface = nullptr;
    VkPhysicalDevice m_PhysicalDevice = nullptr;
    VkDevice m_Device = nullptr;
    
    VkSwapchainKHR m_Swapchain = nullptr;
    std::vector<VulkanTexture*> m_SwapchainImages;

    VkQueue m_GraphicsQueue = nullptr;
    VkQueue m_PresentQueue = nullptr;
    uint32_t m_GraphicsFamily = -1;
    uint32_t m_PresentFamily = -1;

    std::vector<VkSemaphore> m_RenderFinishedSamephores;
    std::vector<VkSemaphore> m_PresentCompleteSemaphores;
    std::vector<VkFence> m_Fences;

    uint64_t m_CurrentFrame = 0;

    uint32_t m_CurrentFrameInFlight = 0;
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

    void RecreateSwapchain();
    void AcquireNextImage();

};

}