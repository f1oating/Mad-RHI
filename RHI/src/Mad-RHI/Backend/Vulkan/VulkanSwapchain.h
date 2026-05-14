#pragma once

#include "Mad-RHI/Swapchain.h"
#include <volk/volk.h>
#include <vector>
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include "Mad-RHI/Backend/Vulkan/VulkanCommandQueue.h"
#include "Mad-RHI/Backend/Vulkan/VulkanFence.h"

namespace mad::rhi {

class VulkanDevice;

class VulkanSwapchain : public ObjectBase<Swapchain>
{
protected:
    ~VulkanSwapchain();

public:
    VulkanSwapchain(VkInstance instance, VkDevice device, VkPhysicalDevice physDevice,
        WindowHandle window, VulkanCommandQueue* commandQueue, VulkanDevice* context);

    virtual Texture* GetCurrentBackBuffer() override;
    virtual void Present() override;

private:
    VulkanDevice* m_Context = nullptr;

    VkInstance m_Instance = nullptr;
    VkPhysicalDevice m_PhysicalDevice = nullptr;
    VkDevice m_Device = nullptr;

    VkSurfaceKHR m_Surface = nullptr;

    VkSwapchainKHR m_Swapchain = nullptr;
    std::vector<VulkanTexture*> m_SwapchainImages;

    uint32_t m_CurrentFrameInFlight = 0;
    uint32_t m_CurrentImageIndex = 0;

    std::vector<VkSemaphore> m_RenderFinishedSamephores;
    std::vector<VkSemaphore> m_PresentCompleteSemaphores;
    std::vector<uint64_t> m_FenceValues;
    VulkanFence* m_Fence = nullptr;

    VulkanCommandQueue* m_CommandQueue = nullptr;

private:
    void CreateSurface(const WindowHandle& wh);
    void CreateSwapchain();
    void DestroySwapchain();
    void CreateFramesInFlightSync();
    void DestroyFramesInFlightSync();

    void RecreateSwapchain();
    void AcquireNextImage();

};

}