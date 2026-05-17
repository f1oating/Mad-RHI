#pragma once

#include "Mad-RHI/Factory.h"
#include <volk/volk.h>
#include <vector>

namespace mad::rhi {

class VulkanFactory : public Factory
{
public:
    VulkanFactory(const FactoryInitInfo& info);
    ~VulkanFactory();

    virtual void EnumerateAdapters(uint32_t& numAdapters, AdapterInfo* adapters) override;
    
    virtual void CreateDevice(Device** ppDevice, const DeviceDesc& desc) override;

    VkInstance GetInstance() { return m_Instance; }
    VkPhysicalDevice GetPhysicalDevice(uint32_t index) { return m_PhysicalDevices[index]; }

private:
    VkInstance m_Instance = nullptr;
    std::vector<VkPhysicalDevice> m_PhysicalDevices;

};

}