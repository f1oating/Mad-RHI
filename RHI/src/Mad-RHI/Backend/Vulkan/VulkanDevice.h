#pragma once

#include "Mad-RHI/Device.h"
#include "Mad-RHI/Factory.h"
#include <volk/volk.h>

namespace mad::rhi {

class VulkanDevice : public ObjectBase<Device>
{
protected:
    ~VulkanDevice();

public:
    VulkanDevice(VkInstance instance, WindowHandle& wh);

private:
    VkInstance m_Instance = nullptr;
    VkSurfaceKHR m_Surface = nullptr;
    VkPhysicalDevice m_PhysicalDevice = nullptr;
    VkDevice m_Device = nullptr;
    VkSwapchainKHR m_Swapchain = nullptr;

    uint32_t m_GraphicsFamily = -1;
    uint32_t m_PresentFamily = -1;

private:
    void CreateSurface(WindowHandle& wh);
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();

};

}