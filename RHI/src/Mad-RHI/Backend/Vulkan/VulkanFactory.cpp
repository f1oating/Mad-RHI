#include "Mad-RHI/Backend/Vulkan/VulkanFactory.h"
#include <iostream>
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"
#include <cstring>

namespace mad::rhi {

VulkanFactory::VulkanFactory(const FactoryInitInfo& info)
{
    volkInitialize();

    VkApplicationInfo vkAppInfo {};
    vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkAppInfo.pApplicationName = info.pAppName;
    vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.pEngineName = info.pEngineName;
    vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.apiVersion = VK_API_VERSION_1_3;

    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
    };

    const char* layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo vkInstInfo {};
    vkInstInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstInfo.pApplicationInfo = &vkAppInfo;
    vkInstInfo.enabledExtensionCount = 2;
    vkInstInfo.ppEnabledExtensionNames = extensions;
    vkInstInfo.enabledLayerCount = 1;
    vkInstInfo.ppEnabledLayerNames = layers;

    vkCreateInstance(&vkInstInfo, nullptr, &m_Instance);
    volkLoadInstance(m_Instance);
    
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
    m_PhysicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, m_PhysicalDevices.data());

    std::cout << "Factory Created" << std::endl;
}

VulkanFactory::~VulkanFactory()
{
    if (m_Instance != nullptr)
    {
        vkDestroyInstance(m_Instance, nullptr);
    }
    std::cout << "Factory Destroyed" << std::endl;
}

void VulkanFactory::EnumerateAdapters(uint32_t& numAdapters, AdapterInfo* adapters)
{
    numAdapters = static_cast<uint32_t>(m_PhysicalDevices.size());
    if (!adapters) return;

    for (uint32_t i = 0; i < numAdapters; i++)
    {
        VkPhysicalDevice pd = m_PhysicalDevices[i];
        AdapterInfo& out = adapters[i];

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(pd, &props);
        strncpy(out.Description, props.deviceName, sizeof(out.Description) - 1);
        out.VendorId = props.vendorID;
        out.DeviceId = props.deviceID;

        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &familyCount, nullptr);
        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &familyCount, families.data());

        out.NumFamilies = 0;
        for (uint32_t f = 0; f < familyCount; f++)
        {
            auto flags = families[f].queueFlags;
            uint8_t typeFlags;

            if (flags & VK_QUEUE_GRAPHICS_BIT) typeFlags |= COMMAND_QUEUE_TYPE_GRAPHICS_BIT;
            else if (flags & VK_QUEUE_COMPUTE_BIT) typeFlags |= COMMAND_QUEUE_TYPE_COMPUTE_BIT;
            else if (flags & VK_QUEUE_TRANSFER_BIT) typeFlags |= COMMAND_QUEUE_TYPE_TRANSFER_BIT;
            else continue;

            QueueFamilyInfo& fi = out.Families[out.NumFamilies++];
            fi.Flags = static_cast<CommandQueueTypeFlags>(typeFlags);
            fi.Index = f;
            fi.MaxQueues = families[f].queueCount;
        }
    }
}

void VulkanFactory::CreateDevice(Device** ppDevice, const DeviceDesc& desc)
{
    *ppDevice = new VulkanDevice(desc, this);
}

}