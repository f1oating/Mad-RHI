#pragma once

#include "Mad-RHI/Device.h"
#include "Mad-RHI/Factory.h"
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include <volk/volk.h>
#include <vector>

namespace mad::rhi {

class VulkanDevice : public ObjectBase<Device>
{
protected:
    ~VulkanDevice();

public:
    VulkanDevice(VkInstance instance, const WindowHandle& wh);

    virtual void Resize() override;

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

private:
    void CreateSurface(const WindowHandle& wh);
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void DestroySwapchain();

};

}