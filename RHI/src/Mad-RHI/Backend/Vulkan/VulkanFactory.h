#pragma once

#include "Mad-RHI/Factory.h"
#include <volk/volk.h>

namespace mad::rhi {

class VulkanFactory : public Factory
{
public:
    VulkanFactory(FactoryInitInfo& info);
    ~VulkanFactory();

    virtual void CreateDevice(Device** ppDevice, ImmidiateCommandList** ppIcl, WindowHandle& wh) override;

private:
    VkInstance m_Instance = nullptr;

};

}