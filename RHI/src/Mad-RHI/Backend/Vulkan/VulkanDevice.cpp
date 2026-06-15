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
    m_PhysicalDevice = factory->GetPhysicalDevice(desc.AdapterId);

    CreateLogicalDevice(desc);
    CreateAllocator();
    m_RingBuffer.Init(m_Allocator);

    std::cout << "VulkanDevice Created" << std::endl;
}

VulkanDevice::~VulkanDevice()
{
    vkDeviceWaitIdle(m_Device);

    for (auto* queue : m_CommandQueues)
    {
        if (queue != nullptr)
        {
            queue->Release();
        }
    }
    m_CommandQueues.clear();

    m_RingBuffer.Shutdown();
    if (m_Allocator) vmaDestroyAllocator(m_Allocator);
    if (m_Device) vkDestroyDevice(m_Device, nullptr);

    std::cout << "Device Destroyed" << std::endl;
}

void VulkanDevice::EndFrame()
{
    VkDeviceSize ringBufferHead = m_RingBuffer.GetHead();
    SafeReleaseResource(new VkRingBufferResource{ ringBufferHead, &m_RingBuffer });

    for (auto* queue : m_CommandQueues)
    {
        queue->EndFrame();
    }

    m_CurrentFrame++;
}

CommandQueue* VulkanDevice::GetCommandQueue(uint32_t index)
{
    return m_CommandQueues[index];
}

void VulkanDevice::CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window, CommandQueue* queue)
{
    VulkanCommandQueue* vulkanQueue = static_cast<VulkanCommandQueue*>(queue);

    *ppSwapchain = new VulkanSwapchain(m_Instance, m_Device, m_PhysicalDevice, window, 
        vulkanQueue, this);
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

    for (auto* queue : m_CommandQueues)
    {
        queue->SafeReleaseResource(wrapper); 
    }

    wrapper.GiveUpOwnership();
}

void VulkanDevice::CreateLogicalDevice(const DeviceDesc& desc)
{
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_Factory->GetPhysicalDevice(desc.AdapterId), &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_Factory->GetPhysicalDevice(desc.AdapterId), &familyCount, families.data());

    std::vector<uint32_t> queueFamilies(desc.NumCommandQueues);
    for (int i = 0; i < desc.NumCommandQueues; i++)
    {
        for (uint32_t f = 0; f < familyCount; f++)
        {
            auto flags = families[f].queueFlags;
            CommandQueueType type;

            if (flags & VK_QUEUE_GRAPHICS_BIT) type = CommandQueueType::COMMAND_QUEUE_TYPE_GRAPHICS;
            else if (flags & VK_QUEUE_COMPUTE_BIT) type = CommandQueueType::COMMAND_QUEUE_TYPE_COMPUTE;
            else if (flags & VK_QUEUE_TRANSFER_BIT) type = CommandQueueType::COMMAND_QUEUE_TYPE_TRANSFER;
            else continue;

            if (desc.pCommandQueues[i].Type == type)
            {
                queueFamilies[i] = f;
                break;
            }
        }
    }

    std::vector<VkDeviceQueueCreateInfo> queueInfos;

    float priority = 1.0f;
    for (uint32_t family : queueFamilies) 
    {
        VkDeviceQueueCreateInfo qi{};
        qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex = family;
        qi.queueCount = 1;
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

    m_CommandQueues.resize(desc.NumCommandQueues);
    for (int i = 0; i < desc.NumCommandQueues; i++)
    {
        VkQueue vkQueue = VK_NULL_HANDLE;
        vkGetDeviceQueue(m_Device, queueFamilies[i], 0, &vkQueue);

        auto* queue = new VulkanCommandQueue(vkQueue, queueFamilies[i], this);
        m_CommandQueues[i] = queue;
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

}