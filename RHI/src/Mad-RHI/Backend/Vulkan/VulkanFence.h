#pragma once

#include "Mad-RHI/Fence.h"
#include <volk/volk.h>
#include "Mad-RHI/Backend/Vulkan/Vk/ReleaseManager.h"

namespace mad::rhi {

class VulkanDevice;

class VulkanFence : public ObjectBase<Fence>
{
protected:
    ~VulkanFence();

public:
    VulkanFence(VulkanDevice* context);

    virtual uint64_t GetCompletedValue() override;
    virtual uint64_t GetCurrentValue() override;
    
    virtual void IncrementCurrentValue() override;

    virtual void Wait(uint64_t value) override;
    
    VkSemaphore GetTimelineSemaphore() { return m_TimelineSemaphore; }

private:
    VulkanDevice* m_Context = nullptr;

    VkSemaphore m_TimelineSemaphore = nullptr;
    uint64_t m_TimelineSemaphoreValue = 0;

};

struct VkTimelineSemaphoreResource : vk::StaleResourceBase
{
    VkSemaphore TimelineSemaphore;
    VkDevice Device;

    VkTimelineSemaphoreResource(VkSemaphore s, VkDevice d) : TimelineSemaphore(s), Device(d) {}

    void Destroy() override { vkDestroySemaphore(Device, TimelineSemaphore, nullptr); }
};

}