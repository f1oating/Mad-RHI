#pragma once

#include "Mad-RHI/Device.h"
#include "Mad-RHI/Factory.h"
#include "Mad-RHI/Backend/Vulkan/VulkanResource.h"
#include <volk/volk.h>
#include <vector>
#include "Mad-RHI/Backend/Vulkan/VulkanCommandQueue.h"
#include <vk_mem_alloc.h>
#include "Mad-RHI/Backend/Vulkan/Vk/RingAllocator.h"
#include "Mad-RHI/Backend/Vulkan/VulkanFactory.h"

namespace mad::rhi {

class VulkanDevice : public ObjectBase<Device>
{
protected:
    ~VulkanDevice();

public:
    VulkanDevice(const DeviceDesc& desc, VulkanFactory* factory);

    virtual void EndFrame() override;
    virtual void GarbageCollect() override;

    virtual RefPtr<CommandQueue> GetCommandQueue() override;

    virtual void CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window) override;
    virtual void CreateTexture(Texture** ppTex, const TextureDesc& desc) override;
    virtual void CreateBuffer(Buffer** ppBuff, const BufferDesc& desc) override;
    virtual void CreateSampler(Sampler** ppSampler, const SamplerDesc& desc) override;
    virtual void CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size) override;
    virtual void CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc) override;
    virtual void CreateFence(Fence** ppFence) override;

    void SafeReleaseResource(vk::StaleResourceBase* resource);

    VkDevice GetDevice() { return m_Device; }
    VmaAllocator GetVmaAllocator() { return m_Allocator; }
    vk::RingBuffer* GetRingBuffer() { return &m_RingBuffer; }

private:
    VulkanFactory* m_Factory = nullptr;

    VkInstance m_Instance = nullptr;
    VkPhysicalDevice m_PhysicalDevice = nullptr;
    VkDevice m_Device = nullptr;

    VkQueue m_GraphicsQueue = nullptr;
    uint32_t m_GraphicsFamily = -1;

    uint64_t m_CurrentFrame = 0;

    RefPtr<VulkanCommandQueue> m_GraphicsCommandQueue = nullptr;

    VmaAllocator m_Allocator = nullptr;

    vk::RingBuffer m_RingBuffer;

private:
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateAllocator();

};

}