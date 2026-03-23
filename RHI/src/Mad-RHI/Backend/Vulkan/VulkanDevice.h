#pragma once

#include "Mad-RHI/Device.h"
#include <volk/volk.h>

namespace mad::rhi {

class VulkanDevice : public ObjectBase<Device>
{
protected:
    ~VulkanDevice();

public:
    VulkanDevice(VkInstance instance, xcb_connection_t* connection, xcb_window_t window);

private:
    VkInstance m_Instance = nullptr;
    VkSurfaceKHR m_Surface = nullptr;
    VkPhysicalDevice m_PhysicalDevice = nullptr;
    VkDevice m_Device = nullptr;
    VkSwapchainKHR m_Swapchain = nullptr;

    uint32_t m_GraphicsFamily = -1;
    uint32_t m_PresentFamily = -1;

private:
    void CreateSurface(xcb_connection_t* connection, xcb_window_t window);
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();

};

}