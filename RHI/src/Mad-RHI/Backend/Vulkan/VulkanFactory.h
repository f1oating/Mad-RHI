#pragma once

#include "Mad-RHI/Factory.h"
#include <volk/volk.h>

namespace mad::rhi {

class VulkanFactory : public Factory
{
public:
    VulkanFactory(const FactoryInitInfo& info);
    ~VulkanFactory();

    virtual void CreateDevice(Device** ppDevice, const WindowHandle& wh) override;

private:
    VkInstance m_Instance = nullptr;

};

}