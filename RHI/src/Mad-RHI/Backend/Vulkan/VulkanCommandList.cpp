#include "Mad-RHI/Backend/Vulkan/VulkanCommandList.h"
#include <iostream>

namespace mad::rhi {

VulkanImmidiateCommandList::VulkanImmidiateCommandList()
{
    std::cout << "VulkanImmidiateCommandList created" << std::endl;
}

VulkanImmidiateCommandList::~VulkanImmidiateCommandList()
{
    std::cout << "VulkanImmidiateCommandList destroyed" << std::endl;
}

}