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

    virtual void CreateDevice(Device** ppDevice, const DeviceDesc& desc) override;

    VkInstance GetInstance() { return m_Instance; }

private:
    VkInstance m_Instance = nullptr;

};

}