#pragma once

#include "Mad-RHI/Swapchain.h"
#include <volk/volk.h>
#include <vector>
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include "Mad-RHI/Backend/Vulkan/VulkanCommandList.h"

namespace mad::rhi {

class VulkanDevice;

class VulkanSwapchain : public ObjectBase<Swapchain>
{
protected:
    ~VulkanSwapchain();

public:
    VulkanSwapchain(VkInstance instance, VkDevice device, VkPhysicalDevice physDevice,
        WindowHandle window, VulkanImmidiateCommandList* immidiateCommandList, VulkanDevice* context);

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
    std::vector<VkFence> m_Fences;

    VulkanImmidiateCommandList* m_ImmidiateCommandList = nullptr;

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