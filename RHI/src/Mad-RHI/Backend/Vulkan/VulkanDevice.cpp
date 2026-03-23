#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

#include <iostream>

namespace mad::rhi {

VulkanDevice::VulkanDevice()
{
    std::cout << "Device created" << std::endl;
}

VulkanDevice::~VulkanDevice()
{
    std::cout << "Device destroyed" << std::endl;
}

}