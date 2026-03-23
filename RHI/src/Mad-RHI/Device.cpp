#include "Mad-RHI/Device.h"
#include "Mad-RHI/Backend/Vulkan/VulkanDevice.h"

namespace mad::rhi {

RefPtr<Device> Device::Create()
{
    return MakeRef<VulkanDevice>();
}

}