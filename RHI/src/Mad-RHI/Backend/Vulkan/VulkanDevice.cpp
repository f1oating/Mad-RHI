#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

#include <iostream>
#include <vector>
#include <set>

namespace mad::rhi {

VulkanDevice::VulkanDevice(VkInstance instance, xcb_connection_t* connection, xcb_window_t window)
{
    m_Instance = instance;

    CreateSurface(connection, window);
    CreatePhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();

    std::cout << "Device created" << std::endl;
}

VulkanDevice::~VulkanDevice()
{
    if (m_Surface) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    if (m_Device) vkDestroyDevice(m_Device, nullptr);

    std::cout << "Device destroyed" << std::endl;
}

void VulkanDevice::CreateSurface(xcb_connection_t* connection, xcb_window_t window)
{
    VkXcbSurfaceCreateInfoKHR info{};
    info.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    info.connection = connection;
    info.window     = window;

    vkCreateXcbSurfaceKHR(m_Instance, &info, nullptr, &m_Surface);
}

void VulkanDevice::CreatePhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (VkPhysicalDevice& pd : devices)
    {
        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueCount, queues.data());

        for (uint32_t i = 0; i < queueCount; i++) 
        {
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                m_GraphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, m_Surface, &presentSupport);
            if (presentSupport)
                m_PresentFamily = i;
        }

        if (m_GraphicsFamily != -1 && m_PresentFamily != -1) 
        {
            m_PhysicalDevice = pd;
            break;
        }
    }
}

void VulkanDevice::CreateLogicalDevice()
{
    std::set<uint32_t> uniqueFamilies = { m_GraphicsFamily, m_PresentFamily };
    std::vector<VkDeviceQueueCreateInfo> queueInfos;

    float priority = 1.0f;
    for (uint32_t family : uniqueFamilies) 
    {
        VkDeviceQueueCreateInfo qi{};
        qi.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex = family;
        qi.queueCount       = 1;
        qi.pQueuePriorities = &priority;
        queueInfos.push_back(qi);
    }

    const char* deviceExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount    = queueInfos.size();
    deviceInfo.pQueueCreateInfos       = queueInfos.data();
    deviceInfo.enabledExtensionCount   = 1;
    deviceInfo.ppEnabledExtensionNames = deviceExts;

    vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device);
}

void VulkanDevice::CreateSwapchain()
{

}

}