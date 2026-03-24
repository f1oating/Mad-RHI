#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

#include <iostream>
#include <set>
#include <algorithm>

namespace mad::rhi {

VulkanDevice::VulkanDevice(VkInstance instance, const WindowHandle& wh)
{
    m_Instance = instance;

    CreateSurface(wh);
    CreatePhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
    CreateFramesInFlightSync();

    AcquireNextImage();

    std::cout << "Device created" << std::endl;
}

VulkanDevice::~VulkanDevice()
{
    vkDeviceWaitIdle(m_Device);

    DestroyFramesInFlightSync();
    DestroySwapchain();
    if (m_Device) vkDestroyDevice(m_Device, nullptr);
    if (m_Surface) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

    std::cout << "Device destroyed" << std::endl;
}

void VulkanDevice::Resize()
{
    vkDeviceWaitIdle(m_Device);
    DestroySwapchain();
    CreateSwapchain();
}

void VulkanDevice::Present()
{
    VkSubmitInfo si{};
    si.sType                 = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.signalSemaphoreCount  = 1;
    si.pSignalSemaphores     = &m_RenderFinishedSamephores[m_CurrentImageIndex];
    si.commandBufferCount    = 0;
    si.pCommandBuffers       = nullptr;
    vkQueueSubmit(m_GraphicsQueue, 1, &si, m_Fences[m_CurrentFrame]);

    VkPresentInfoKHR pi{};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores    = &m_RenderFinishedSamephores[m_CurrentFrame];
    pi.swapchainCount = 1;
    pi.pSwapchains    = &m_Swapchain;
    pi.pImageIndices  = &m_CurrentImageIndex;

    vkQueuePresentKHR(m_PresentQueue, &pi);
    m_CurrentFrame = (m_CurrentFrame + 1) % 2;
    AcquireNextImage();
}

void VulkanDevice::CreateSurface(const WindowHandle& wh)
{
    switch (wh.platform)
    {
        case WindowHandle::Platform::XCB:
        {
            VkXcbSurfaceCreateInfoKHR info{};
            info.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            info.connection = static_cast<xcb_connection_t*>(wh.xcb.connection);
            info.window     = static_cast<xcb_window_t>(wh.xcb.window);

            vkCreateXcbSurfaceKHR(m_Instance, &info, nullptr, &m_Surface);
        }
    }
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

    VkPhysicalDeviceTimelineSemaphoreFeatures tf{};
    tf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
    tf.timelineSemaphore = VK_TRUE;

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext                   = &tf;
    deviceInfo.queueCreateInfoCount    = queueInfos.size();
    deviceInfo.pQueueCreateInfos       = queueInfos.data();
    deviceInfo.enabledExtensionCount   = 1;
    deviceInfo.ppEnabledExtensionNames = deviceExts;

    vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device);

    vkGetDeviceQueue(m_Device, m_GraphicsFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentFamily, 0, &m_PresentQueue);
}

void VulkanDevice::CreateSwapchain()
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &caps);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, formats.data());

    VkSurfaceFormatKHR chosenFormat = formats[0];
    for (auto& f : formats)
    {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            chosenFormat = f;
    }

    uint32_t modeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &modeCount, nullptr);
    std::vector<VkPresentModeKHR> modes(modeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &modeCount, modes.data());

    VkPresentModeKHR chosenMode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto& m : modes)
    {
        if (m == VK_PRESENT_MODE_MAILBOX_KHR)
            chosenMode = m;
    }        

    uint32_t imageCount = 3;

    VkSwapchainCreateInfoKHR info{};
    info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface          = m_Surface;
    info.minImageCount    = imageCount;
    info.imageFormat      = chosenFormat.format;
    info.imageColorSpace  = chosenFormat.colorSpace;
    info.imageExtent      = caps.currentExtent;
    info.imageArrayLayers = 1;
    info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.preTransform     = caps.currentTransform;
    info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode      = chosenMode;
    info.clipped          = VK_TRUE;
    info.oldSwapchain     = nullptr;

    if (m_GraphicsFamily != m_PresentFamily) 
    {
        uint32_t families[] = { m_GraphicsFamily, m_PresentFamily };
        info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices   = families;
    } else 
    {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    vkCreateSwapchainKHR(m_Device, &info, nullptr, &m_Swapchain);

    uint32_t imagesCount;
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imagesCount, nullptr);
    std::vector<VkImage> images(imagesCount);
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imagesCount, images.data());

    m_SwapchainImages.resize(imagesCount);
    for (int i = 0; i < imagesCount; i++)
    {
        TextureDesc texDesc{};
        texDesc.Name            = "Swapchain image";
        texDesc.Dimension       = TextureDimension::Texture2D;
        texDesc.Width           = caps.currentExtent.width;
        texDesc.Height          = caps.currentExtent.height;
        texDesc.ArraySize       = 1;
        texDesc.Format          = FromVkFormat(chosenFormat.format);
        texDesc.MipLevels       = 1;
        texDesc.SampleCount     = 1;
        texDesc.BindFlags       = RenderTarget;
        texDesc.Usage           = ResourceUsage::Default;
        m_SwapchainImages[i]    = MakeRef<VulkanTexture>(texDesc, images[i]);
    }
}

void VulkanDevice::DestroySwapchain()
{
    if (m_Swapchain)
    {
        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        m_SwapchainImages.clear();
    }
}

void VulkanDevice::CreateFramesInFlightSync()
{
    m_RenderFinishedSamephores.resize(m_SwapchainImages.size());
    m_PresentCompleteSemaphores.resize(m_SwapchainImages.size());
    m_Fences.resize(2);

    for (int i = 0; i < m_SwapchainImages.size(); i++)
    {
        VkSemaphoreCreateInfo sci{};
        sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(m_Device, &sci, nullptr, &m_RenderFinishedSamephores[i]);
    }
    for (int i = 0; i < 2; i++)
    {
        VkSemaphoreCreateInfo sci{};
        sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(m_Device, &sci, nullptr, &m_PresentCompleteSemaphores[i]);

        VkFenceCreateInfo fci{};
        fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(m_Device, &fci, nullptr, &m_Fences[i]);
    }
}

void VulkanDevice::DestroyFramesInFlightSync()
{
    for (auto& sem : m_RenderFinishedSamephores)
    {
        if (sem != nullptr)
        {
            vkDestroySemaphore(m_Device, sem, nullptr);
        }
    }
    for (auto& sem : m_PresentCompleteSemaphores)
    {
        if (sem != nullptr)
        {
            vkDestroySemaphore(m_Device, sem, nullptr);
        }
    }
    for (auto& fence : m_Fences)
    {
        if (fence != nullptr)
        {
            vkDestroyFence(m_Device, fence, nullptr);
        }
    }
}

void VulkanDevice::AcquireNextImage()
{
    vkWaitForFences(m_Device, 1, &m_Fences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_Device, 1, &m_Fences[m_CurrentFrame]);
    vkAcquireNextImageKHR(
        m_Device, m_Swapchain, UINT64_MAX, m_PresentCompleteSemaphores[m_CurrentFrame],
        nullptr, &m_CurrentImageIndex
    );

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si{};
    si.sType               = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.pWaitDstStageMask   = &waitStage;
    si.waitSemaphoreCount  = 1;
    si.pWaitSemaphores     = &m_PresentCompleteSemaphores[m_CurrentFrame];
    si.commandBufferCount  = 0;
    si.pCommandBuffers     = nullptr;
    vkQueueSubmit(m_GraphicsQueue, 1, &si, nullptr);
}

}