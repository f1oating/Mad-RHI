#pragma once

#include "Mad-RHI/Factory.h"
#include <volk/volk.h>

namespace mad::rhi {

class VulkanFactory : public Factory
{
public:
    VulkanFactory(FactoryInitInfo& info);
    ~VulkanFactory();

private:
    VkInstance m_Instance = nullptr;

};

}