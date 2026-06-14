#include "Mad-RHI/Backend/DX12/DX12Resource.h"

namespace mad::rhi {

DX12Texture::DX12Texture(const TextureDesc& desc)
{

}

DX12Texture::~DX12Texture()
{

}

RefPtr<TextureView> DX12Texture::GetDefaultSRV()
{

}

RefPtr<TextureView> DX12Texture::GetDefaultRTV()
{

}

RefPtr<TextureView> DX12Texture::GetDefaultDSV()
{

}

const TextureDesc& DX12Texture::GetDesc()
{

}

ResourceState DX12Texture::GetCurrentResourceState()
{

}

DX12Buffer::DX12Buffer(const BufferDesc& desc)
{

}

DX12Buffer::~DX12Buffer()
{

}

void* DX12Buffer::Map()
{

}

void DX12Buffer::Unmap()
{

}
    
const BufferDesc& DX12Buffer::GetDesc()
{

}

ResourceState DX12Buffer::GetCurrentResourceState()
{

}

DX12Sampler::DX12Sampler(const SamplerDesc& desc)
{

}

DX12Sampler::~DX12Sampler()
{

}

const SamplerDesc& DX12Sampler::GetDesc()
{

}

DX12TextureView::DX12TextureView(RefCounter* sharedCounter, const TextureViewDesc& desc)
{

}

DX12TextureView::~DX12TextureView()
{

}

}