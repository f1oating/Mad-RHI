#include "Mad-RHI/Backend/Vulkan/VulkanCommandList.h"
#include <iostream>

namespace mad::rhi {

VulkanGraphicsImmidiateCommandList::VulkanGraphicsImmidiateCommandList()
{
    std::cout << "VulkanGraphicsImmidiateCommandList created" << std::endl;
}

VulkanGraphicsImmidiateCommandList::~VulkanGraphicsImmidiateCommandList()
{
    std::cout << "VulkanGraphicsImmidiateCommandList destroyed" << std::endl;
}

}