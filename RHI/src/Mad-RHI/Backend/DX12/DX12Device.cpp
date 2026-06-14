#include "Mad-RHI/Backend/DX12/DX12Device.h"

namespace mad::rhi {

DX12Device::DX12Device(const DeviceDesc& desc)
{

}

DX12Device::~DX12Device()
{

}

void DX12Device::EndFrame()
{

}

CommandQueue* DX12Device::GetCommandQueue(uint32_t index)
{

}

void DX12Device::CreateSwapchain(Swapchain** ppSwapchain, WindowHandle window, CommandQueue* queue)
{

}

void DX12Device::CreateTexture(Texture** ppTex, const TextureDesc& desc)
{

}

void DX12Device::CreateBuffer(Buffer** ppBuff, const BufferDesc& desc)
{

}

void DX12Device::CreateSampler(Sampler** ppSampler, const SamplerDesc& desc)
{

}

void DX12Device::CreateShader(Shader** ppShader, const uint32_t* data, uint64_t size)
{

}

void DX12Device::CreateGraphicsPipeline(GraphicsPipelineState** ppPipeline, const GraphicsPipelineDesc& desc)
{

}

void DX12Device::CreateFence(Fence** ppFence)
{

}

}