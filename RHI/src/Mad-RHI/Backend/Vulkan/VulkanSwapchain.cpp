#include "Mad-RHI/Backend/Vulkan/VulkanSwapchain.h"
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

namespace mad::rhi {
    
VulkanSwapchain::VulkanSwapchain(VkInstance instance, VkDevice device, VkPhysicalDevice physDevice,
    WindowHandle window, VulkanCommandQueue* commandQueue, VulkanDevice* context)
{
    m_Instance = instance;
    m_Device = device;
    m_PhysicalDevice = physDevice;
    m_CommandQueue = commandQueue;
    m_Context = context;

    m_FenceValues.resize(2);

    CreateSurface(window);
    CreateSwapchain();
    CreateFramesInFlightSync();
    m_Fence = new VulkanFence(m_Context);

    AcquireNextImage();
}

VulkanSwapchain::~VulkanSwapchain()
{
    vkDeviceWaitIdle(m_Device);

    m_Fence->Release();
    DestroyFramesInFlightSync();
    DestroySwapchain();

    if (m_Surface) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}

void VulkanSwapchain::Resize()
{
    RecreateSwapchain();
}

void VulkanSwapchain::Present()
{
    m_FenceValues[m_CurrentFrameInFlight] = m_Fence->IncrementCurrentValue();
    m_CommandQueue->AddSignalSemaphore(m_RenderFinishedSamephores[m_CurrentImageIndex]);
    m_CommandQueue->EnqueueSignal(m_Fence, m_Fence->GetCurrentValue());
    m_CommandQueue->Flush();

    VkPresentInfoKHR pi{};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &m_RenderFinishedSamephores[m_CurrentImageIndex];
    pi.swapchainCount = 1;
    pi.pSwapchains = &m_Swapchain;
    pi.pImageIndices = &m_CurrentImageIndex;

    VkResult res = vkQueuePresentKHR(m_CommandQueue->GetQueue(), &pi);
    if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return;
    }

    m_CurrentFrameInFlight = (m_CurrentFrameInFlight + 1) % 2;
    AcquireNextImage();
}

Texture* VulkanSwapchain::GetCurrentBackBuffer()
{
    return m_SwapchainImages[m_CurrentImageIndex];
}

void VulkanSwapchain::CreateSurface(const WindowHandle& wh)
{
    switch (wh.platform)
    {
        #ifdef _WIN32
            case WindowHandle::Platform::WIN:
            {
                VkWin32SurfaceCreateInfoKHR info{};
                info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
                info.hwnd = static_cast<HWND>(wh.win32.hwnd);
                info.hinstance = static_cast<HINSTANCE>(wh.win32.hinstance);

                vkCreateWin32SurfaceKHR(m_Instance, &info, nullptr, &m_Surface);
            }
        #else
            case WindowHandle::Platform::XCB:
            {
                VkXcbSurfaceCreateInfoKHR info{};
                info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
                info.connection = static_cast<xcb_connection_t*>(wh.xcb.connection);
                info.window = static_cast<xcb_window_t>(wh.xcb.window);

                vkCreateXcbSurfaceKHR(m_Instance, &info, nullptr, &m_Surface);
            }
        #endif
    }
}

void VulkanSwapchain::CreateSwapchain()
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

    uint32_t imageCount = caps.minImageCount + 1;

    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.surface = m_Surface;
    info.minImageCount = imageCount;
    info.imageFormat = chosenFormat.format;
    info.imageColorSpace = chosenFormat.colorSpace;
    info.imageExtent = caps.currentExtent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.preTransform = caps.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = chosenMode;
    info.clipped = VK_TRUE;
    info.oldSwapchain = nullptr;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateSwapchainKHR(m_Device, &info, nullptr, &m_Swapchain);

    uint32_t imagesCount;
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imagesCount, nullptr);
    std::vector<VkImage> images(imagesCount);
    vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imagesCount, images.data());

    m_SwapchainImages.resize(imagesCount);
    for (int i = 0; i < imagesCount; i++)
    {
        TextureDesc texDesc{};
        texDesc.Dimension = TextureDimension::Texture2D;
        texDesc.Width = caps.currentExtent.width;
        texDesc.Height = caps.currentExtent.height;
        texDesc.ArraySize = 1;
        texDesc.Format = FromVkFormat(chosenFormat.format);
        texDesc.MipLevels = 1;
        texDesc.SampleCount = 1;
        texDesc.BindFlags = RESOURCE_BIND_RENDER_TARGET;
        texDesc.Usage = ResourceUsage::Default;
        m_SwapchainImages[i] = new VulkanTexture(texDesc, images[i], m_Context);
    }
}

void VulkanSwapchain::DestroySwapchain()
{
    if (m_Swapchain)
    {
        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        for (auto image : m_SwapchainImages)
        {
            image->Release();
        }
        m_SwapchainImages.clear();
    }
}

void VulkanSwapchain::CreateFramesInFlightSync()
{
    m_RenderFinishedSamephores.resize(m_SwapchainImages.size());
    m_PresentCompleteSemaphores.resize(2);

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
    }
}

void VulkanSwapchain::DestroyFramesInFlightSync()
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
}

void VulkanSwapchain::RecreateSwapchain()
{
    vkDeviceWaitIdle(m_Device);
    m_FenceValues[m_CurrentFrameInFlight] = m_Fence->IncrementCurrentValue();
    m_CommandQueue->EnqueueSignal(m_Fence, m_Fence->GetCurrentValue());
    m_CommandQueue->Flush();
    m_Fence->Wait(m_FenceValues[m_CurrentFrameInFlight]);

    DestroyFramesInFlightSync();
    DestroySwapchain();

    CreateSwapchain();
    CreateFramesInFlightSync();
    
    AcquireNextImage();
}

void VulkanSwapchain::AcquireNextImage()
{
    m_Fence->Wait(m_FenceValues[m_CurrentFrameInFlight]);
    VkResult res = vkAcquireNextImageKHR(
        m_Device, m_Swapchain, UINT64_MAX, m_PresentCompleteSemaphores[m_CurrentFrameInFlight],
        nullptr, &m_CurrentImageIndex
    );

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return;
    }

    m_CommandQueue->AddWaitSemaphore(m_PresentCompleteSemaphores[m_CurrentFrameInFlight]);
}

}