#include "Mad-RHI/Backend/Vulkan/VulkanFactory.h"
#include <iostream>
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

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
        VK_KHR_XCB_SURFACE_EXTENSION_NAME
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

void VulkanFactory::CreateDevice(Device** ppDevice, const WindowHandle& wh)
{
    *ppDevice = new VulkanDevice(m_Instance, wh);
}

}