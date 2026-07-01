#include "Mad-RHI/Backend/DX12/DX12Resource.h"
#include "Mad-RHI/Backend/DX12/DX12Device.h"

namespace mad::rhi {

DX12Texture::DX12Texture(const TextureDesc& desc, DX12Device* context)
{
    m_Context = context;
    m_Desc = desc;

    D3D12_RESOURCE_DESC1 resourceDesc = {};
    resourceDesc.Dimension = ToDXGIResourceDimension(m_Desc.Dimension);
    resourceDesc.Alignment = 0;
    resourceDesc.Width = m_Desc.Width;
    resourceDesc.Height = m_Desc.Height;
    resourceDesc.DepthOrArraySize = m_Desc.Dimension == TextureDimension::Texture3D ? m_Desc.Depth : m_Desc.ArraySize;
    resourceDesc.MipLevels = m_Desc.MipLevels;
    resourceDesc.Format = ToDXGIFormat(m_Desc.Format);
    resourceDesc.SampleDesc.Count = m_Desc.SampleCount;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = ToD3D12ResourceFlags(m_Desc.BindFlags);
    
    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    
    D3D12_CLEAR_VALUE clearValue{ ToDXGIFormat(m_Desc.Format), { 0, 0, 0, 1 } };
    D3D12_CLEAR_VALUE* pClearValue = nullptr;

    if (m_Desc.BindFlags & RESOURCE_BIND_RENDER_TARGET || m_Desc.BindFlags & RESOURCE_BIND_DEPTH_STENCIL)
    {
        pClearValue = &clearValue;
    }

    m_Context->GetAllocator()->CreateResource3(
        &allocationDesc,
        &resourceDesc,
        D3D12_BARRIER_LAYOUT_UNDEFINED,
        pClearValue, 0, nullptr,
        &m_Allocation,
        IID_PPV_ARGS(&m_Resource));
}

DX12Texture::DX12Texture(ID3D12Resource* res, const TextureDesc& desc, DX12Device* context)
{
    m_Context = context;
    m_Resource = res;
    m_Desc = desc;
}

DX12Texture::~DX12Texture()
{
    if (m_Allocation)
    {
        m_Allocation->Release();
    } else
    {
        m_Resource->Release();
    }
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
    return m_Desc;
}

ResourceState DX12Texture::GetCurrentResourceState()
{
    return m_CurrentState;
}

DX12Buffer::DX12Buffer(const BufferDesc& desc, DX12Device* context)
{
    m_Context = context;
    m_Desc = desc;

    D3D12_RESOURCE_DESC1 resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = m_Desc.Size;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = ToD3D12ResourceFlags(m_Desc.BindFlags);

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    m_Context->GetAllocator()->CreateResource3(
        &allocationDesc,
        &resourceDesc,
        D3D12_BARRIER_LAYOUT_UNDEFINED,
        nullptr, 0, nullptr,
        &m_Allocation,
        IID_PPV_ARGS(&m_Resource));
}

DX12Buffer::~DX12Buffer()
{
    if (m_Allocation)
    {
        m_Allocation->Release();
    }
}

void* DX12Buffer::Map()
{

}

void DX12Buffer::Unmap()
{

}
    
const BufferDesc& DX12Buffer::GetDesc()
{
    return m_Desc;
}

ResourceState DX12Buffer::GetCurrentResourceState()
{
    return m_CurrentState;
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