#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

#include <iostream>
#include <set>
#include <algorithm>
#include "Mad-RHI/Backend/Vulkan/VulkanPipelineState.h"

namespace mad::rhi {

VulkanDevice::VulkanDevice(VkInstance instance, const WindowHandle& wh)
{
    m_Instance = instance;

    CreateSurface(wh);
    CreatePhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
    CreateFramesInFlightSync();
    CreateAllocator();
    m_RingBuffer.Init(m_Allocator);

    m_GraphicsImmidiateCommandList = MakeRef<VulkanImmidiateCommandList>(this);

    AcquireNextImage();

    std::cout << "Device created" << std::endl;
}

VulkanDevice::~VulkanDevice()
{
    vkDeviceWaitIdle(m_Device);

    m_GraphicsImmidiateCommandList = nullptr;

    m_RingBuffer.Shutdown();
    if (m_Allocator) vmaDestroyAllocator(m_Allocator);
    DestroyFramesInFlightSync();
    DestroySwapchain();
    if (m_Device) vkDestroyDevice(m_Device, nullptr);
    if (m_Surface) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

    std::cout << "Device destroyed" << std::endl;
}

void VulkanDevice::Resize()
{
    vkDeviceWaitIdle(m_Device);

    DestroyFramesInFlightSync();
    DestroySwapchain();

    CreateSwapchain();
    CreateFramesInFlightSync();

    m_GraphicsImmidiateCommandList->FlushWaitSemaphores();
    m_GraphicsImmidiateCommandList->FlushSignalSemaphores();

    AcquireNextImage();
}

void VulkanDevice::EndFrame()
{
    VkDeviceSize ringBufferHead = m_RingBuffer.GetHead();
    SafeReleaseResource(new VkRingBufferResource{ ringBufferHead, &m_RingBuffer });

    m_GraphicsImmidiateCommandList->EndFrame();
}

void VulkanDevice::GarbageCollect()
{
    m_GraphicsImmidiateCommandList->GarbageCollect();
}

void VulkanDevice::Present()
{
    TextureBarrier tb{};
    tb.NewState = ResourceState::Present;
    tb.Texture = m_SwapchainImages[m_CurrentImageIndex];
    m_GraphicsImmidiateCommandList->ResourceBarrier({ tb }, {});
    m_GraphicsImmidiateCommandList->AddSignalSemaphore(m_RenderFinishedSamephores[m_CurrentImageIndex]);
    m_GraphicsImmidiateCommandList->Flush();

    VkSubmitInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkQueueSubmit(m_GraphicsQueue, 1, &si, m_Fences[m_CurrentFrame]);

    VkPresentInfoKHR pi{};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &m_RenderFinishedSamephores[m_CurrentImageIndex];
    pi.swapchainCount = 1;
    pi.pSwapchains = &m_Swapchain;
    pi.pImageIndices = &m_CurrentImageIndex;

    VkResult res = vkQueuePresentKHR(m_PresentQueue, &pi);
    if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        Resize();
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % 2;
    AcquireNextImage();
}

RefPtr<ImmidiateCommandList> VulkanDevice::GetImmidiateCommandList()
{
    return m_GraphicsImmidiateCommandList;
}

RefPtr<Texture> VulkanDevice::CreateTexture(const TextureDesc& desc)
{
    VkImageType imageType;
    switch (desc.Dimension)
    {
    case TextureDimension::Texture1D:
    case TextureDimension::Texture1DArray:
        imageType = VK_IMAGE_TYPE_1D; break;
    case TextureDimension::Texture3D:
        imageType = VK_IMAGE_TYPE_3D; break;
    default:
        imageType = VK_IMAGE_TYPE_2D; break;
    }

    bool isCube = (desc.Dimension == TextureDimension::TextureCube ||
        desc.Dimension == TextureDimension::TextureCubeArray);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (desc.BindFlags & RenderTarget) usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (desc.BindFlags & DepthStencil) usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (desc.BindFlags & ShaderResource) usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (desc.BindFlags & UnorderedAccess) usage |= VK_IMAGE_USAGE_STORAGE_BIT;

    VkImageCreateInfo ici{};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = imageType;
    ici.format = ToVkFormat(desc.Format);
    ici.extent = { desc.Width, desc.Height, (imageType == VK_IMAGE_TYPE_3D) ? desc.Depth : 1 };
    ici.mipLevels = desc.MipLevels;
    ici.arrayLayers = isCube ? desc.ArraySize * 6 : desc.ArraySize;
    ici.samples = static_cast<VkSampleCountFlagBits>(desc.SampleCount);
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = usage;
    ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (isCube) ici.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VkImage image;
    VmaAllocation allocation;
    vmaCreateImage(m_Allocator, &ici, &aci, &image, &allocation, nullptr);

    return MakeRef<VulkanTexture>(desc, image, allocation, m_Allocator, this);
}

RefPtr<Buffer> VulkanDevice::CreateBuffer(const BufferDesc& desc)
{
    if (desc.Usage == ResourceUsage::Dynamic)
    {
        return MakeRef<VulkanBuffer>(desc, m_RingBuffer.GetBuffer(), m_RingBuffer.GetMappedPtr(), this);
    }

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (desc.BindFlags & VertexBuffer) usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (desc.BindFlags & IndexBuffer) usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (desc.BindFlags & UniformBuffer) usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (desc.BindFlags & ShaderResource) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (desc.BindFlags & UnorderedAccess) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBufferCreateInfo bci{};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = desc.Size;
    bci.usage = usage;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo aci{};
    if (desc.Usage == ResourceUsage::Readback)
    {
        aci.usage = VMA_MEMORY_USAGE_AUTO;
        aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        bci.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    else
    {
        aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(m_Allocator, &bci, &aci, &buffer, &allocation, nullptr);

    return MakeRef<VulkanBuffer>(desc, buffer, allocation, m_Allocator, this);
}

RefPtr<Shader> VulkanDevice::CreateShader(const uint32_t* data, uint64_t size)
{
    return MakeRef<VulkanShader>(m_Device, data, size);
}

RefPtr<GraphicsPipelineState> VulkanDevice::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc)
{
    return MakeRef<VulkanGraphicsPipelineState>(m_Device, desc, this);
}

void VulkanDevice::SafeReleaseResource(vk::StaleResourceBase* resource)
{
    auto wrapper = vk::StaleResourceWrapper::Create(resource, 1);

    m_GraphicsImmidiateCommandList->SafeReleaseResource(wrapper); 

    wrapper.GiveUpOwnership();
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

    VkPhysicalDeviceVulkan13Features vk13{};
    vk13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vk13.pNext = nullptr;
    vk13.dynamicRendering = VK_TRUE;
    vk13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceVulkan12Features vk12{};
    vk12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vk12.pNext = &vk13;
    vk12.timelineSemaphore = VK_TRUE;

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType  = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext  = &vk12;
    deviceInfo.queueCreateInfoCount = queueInfos.size();
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.enabledExtensionCount = 1;
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

    if (m_GraphicsFamily != m_PresentFamily) 
    {
        uint32_t families[] = { m_GraphicsFamily, m_PresentFamily };
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = families;
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
        texDesc.Name = "Swapchain image";
        texDesc.Dimension = TextureDimension::Texture2D;
        texDesc.Width = caps.currentExtent.width;
        texDesc.Height = caps.currentExtent.height;
        texDesc.ArraySize = 1;
        texDesc.Format = FromVkFormat(chosenFormat.format);
        texDesc.MipLevels = 1;
        texDesc.SampleCount = 1;
        texDesc.BindFlags = RenderTarget;
        texDesc.Usage = ResourceUsage::Default;
        m_SwapchainImages[i] = MakeRef<VulkanTexture>(texDesc, images[i]);
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
    m_PresentCompleteSemaphores.resize(2);
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

void VulkanDevice::CreateAllocator()
{
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo info = {};
    info.physicalDevice = m_PhysicalDevice;
    info.device = m_Device;
    info.instance = m_Instance;
    info.vulkanApiVersion = VK_API_VERSION_1_3;
    info.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&info, &m_Allocator);
}

void VulkanDevice::AcquireNextImage()
{
    vkWaitForFences(m_Device, 1, &m_Fences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
    VkResult res = vkAcquireNextImageKHR(
        m_Device, m_Swapchain, UINT64_MAX, m_PresentCompleteSemaphores[m_CurrentFrame],
        nullptr, &m_CurrentImageIndex
    );

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        Resize();
        return;
    }

    vkResetFences(m_Device, 1, &m_Fences[m_CurrentFrame]);
    m_GraphicsImmidiateCommandList->AddWaitSemaphore(m_PresentCompleteSemaphores[m_CurrentFrame]);
}

}