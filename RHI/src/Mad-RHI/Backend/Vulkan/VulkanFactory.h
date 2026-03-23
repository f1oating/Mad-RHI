#pragma once

#include "Mad-RHI/Factory.h"
#include <volk/volk.h>

namespace mad::rhi {

class VulkanFactory : public Factory
{
public:
    VulkanFactory(FactoryInitInfo& info);
    ~VulkanFactory();

    virtual void CreateDevice(Device** ppDevice, ImmidiateCommandList** ppIcl, xcb_connection_t* connection, xcb_window_t window) override;

private:
    VkInstance m_Instance = nullptr;

};

}