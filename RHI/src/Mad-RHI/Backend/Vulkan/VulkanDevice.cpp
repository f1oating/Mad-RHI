#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

#include <iostream>
#include <set>
#include <algorithm>
#include "Mad-RHI/Backend/Vulkan/VulkanPipelineState.h"
#include "Mad-RHI/Backend/Vulkan/VulkanFence.h"
#include "Mad-RHI/Backend/Vulkan/VulkanSwapchain.h"

namespace mad::rhi {

VulkanDevice::VulkanDevice(const DeviceDesc& desc, VulkanFactory* factory)
{
    m_Factory = factory;
    
    m_Instance = factory->GetInstance();

    CreatePhysicalDevice();
    CreateLogicalDevice();
    CreateAllocator();
    m_RingBuffer.Init(m_Allocator);

    m_GraphicsCommandQueue = MakeRef<VulkanCommandQueue>(m_GraphicsQueue, m_GraphicsFamily, this);

    std::cout << "Device created" << std::endl;
}

VulkanDevice::~VulkanDevice()
{
    vkDeviceWaitIdle(m_Device);

    m_GraphicsCommandQueue = nullptr;

    m_RingBuffer.Shutdown();
    if (m_Allocator) vmaDestroyAllocator(m_Allocator);
    if (m_Device) vkDestroyDevice(m_Device, nullptr);

    std::cout << "Device destroyed" << std::endl;
}

void VulkanDevice::EndFrame()
{
    VkDeviceSize ringBufferHead = m_RingBuffer.GetHead();
    SafeReleaseResource(new VkRingBufferResource{ ringBufferHead, &m_RingBuffer });

    m_GraphicsCommandQueue->EndFrame();

    m_CurrentFrame++;
}

void VulkanDevice::GarbageCollect()
{
    m_GraphicsCommandQueue->GarbageCollect();
}

RefPtr<CommandQueue> VulkanDevice::GetCommandQueue()
{
    return m_GraphicsCommandQueue;
}

void VulkanDevice::CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window)
{
    *ppSwapchain = new VulkanSwapchain(m_Instance, m_Device, m_PhysicalDevice, window, 
        m_GraphicsCommandQueue.Get(), this);
}

void VulkanDevice::CreateTexture(Texture** ppTex, const TextureDesc& desc)
{
    *ppTex = new VulkanTexture(desc, this);
}

void VulkanDevice::CreateBuffer(Buffer** ppBuff, const BufferDesc& desc)
{
    *ppBuff = new VulkanBuffer(desc, this);
}

void VulkanDevice::CreateSampler(Sampler** ppSampler, const SamplerDesc& desc)
{
    *ppSampler = new VulkanSampler(desc, this);
}

void VulkanDevice::CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size)
{
    *ppShader = new VulkanShader(m_Device, data, size);
}

void VulkanDevice::CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc)
{
    *ppPipeline = new VulkanGraphicsPipelineState(desc, this);
}

void VulkanDevice::CreateFence(Fence** ppFence)
{
    *ppFence = new VulkanFence(this);
}

void VulkanDevice::SafeReleaseResource(vk::StaleResourceBase* resource)
{
    auto wrapper = vk::StaleResourceWrapper::Create(resource, 1);

    m_GraphicsCommandQueue->SafeReleaseResource(wrapper); 

    wrapper.GiveUpOwnership();
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
        }

        if (m_GraphicsFamily != -1) 
        {
            m_PhysicalDevice = pd;
            break;
        }
    }
}

void VulkanDevice::CreateLogicalDevice()
{
    std::set<uint32_t> uniqueFamilies = { m_GraphicsFamily };
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

}