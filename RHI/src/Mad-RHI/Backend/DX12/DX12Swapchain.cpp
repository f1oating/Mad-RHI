#include "Mad-RHI/Backend/DX12/DX12Swapchain.h"
#include "Mad-RHI/Backend/DX12/DX12Device.h"

namespace mad::rhi {

DX12Swapchain::DX12Swapchain(WindowHandle window, DX12CommandQueue* queue, DX12Device* context)
{
    m_Context = context;
    m_Queue = queue;
    m_Window = window;

    RECT windowRect {};
    GetClientRect((HWND)window.win32.hwnd, &windowRect);

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc {};
    swapchainDesc.Width = (UINT) (windowRect.right - windowRect.left);
    swapchainDesc.Height = (UINT) (windowRect.bottom - windowRect.top);
    swapchainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchainDesc.Stereo = false;
    swapchainDesc.SampleDesc = { 1, 0 };
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = 3;
    swapchainDesc.Scaling = DXGI_SCALING_NONE;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = 0;

    IDXGISwapChain1* swapchain = nullptr;
    m_Context->GetFactory()->GetFactory()->CreateSwapChainForHwnd(m_Queue->GetQueue(), (HWND)window.win32.hwnd, 
        &swapchainDesc, nullptr, nullptr, &swapchain);

    swapchain->QueryInterface(&m_Swapchain);

    m_Backbuffers.resize(3);
    for (int i = 0; i < 3; i++)
    {
        ID3D12Resource* renderTarget = nullptr;
        m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&renderTarget));

        TextureDesc renderTargetDesc {};
        renderTargetDesc.ArraySize = 1;
        renderTargetDesc.BindFlags = RESOURCE_BIND_RENDER_TARGET;
        renderTargetDesc.Dimension = TextureDimension::Texture2D;
        renderTargetDesc.Format = TextureFormat::B8G8R8A8_UNorm;
        renderTargetDesc.Width = (UINT) (windowRect.right - windowRect.left);
        renderTargetDesc.Height = (UINT) (windowRect.bottom - windowRect.top);
        renderTargetDesc.SampleCount = 1;
        renderTargetDesc.MipLevels = 1;
        renderTargetDesc.Usage = ResourceUsage::Default;

        m_Backbuffers[i] = new DX12Texture(renderTarget, renderTargetDesc, m_Context);
    }

    m_Fence = new DX12Fence(m_Context);

    m_CurrentBackbuffer = m_Swapchain->GetCurrentBackBufferIndex();
}

DX12Swapchain::~DX12Swapchain()
{
    for (auto* buffer : m_Backbuffers)
    {
        if (buffer)
        {
            buffer->Release();
        }
    }
    m_Backbuffers.clear();

    if (m_Swapchain)
    {
        m_Swapchain->Release();
    }
}

void DX12Swapchain::Resize()
{
    uint64_t value = m_Fence->IncrementCurrentValue();
    m_Queue->GetQueue()->Signal(m_Fence->GetFence(), value);

    m_Fence->Wait(value);

    for (auto* buffer : m_Backbuffers)
    {
        if (buffer)
        {
            buffer->Release();
        }
    }
    m_Backbuffers.clear();

    RECT windowRect {};
    GetClientRect((HWND) m_Window.win32.hwnd, &windowRect);

    m_Swapchain->ResizeBuffers(3, (UINT) (windowRect.right - windowRect.left),
        (UINT) (windowRect.bottom - windowRect.top), DXGI_FORMAT_B8G8R8A8_UNORM, 0);

    m_Backbuffers.resize(3);
    for (int i = 0; i < 3; i++)
    {
        ID3D12Resource* renderTarget = nullptr;
        m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&renderTarget));

        TextureDesc renderTargetDesc {};
        renderTargetDesc.ArraySize = 1;
        renderTargetDesc.BindFlags = RESOURCE_BIND_RENDER_TARGET;
        renderTargetDesc.Dimension = TextureDimension::Texture2D;
        renderTargetDesc.Format = TextureFormat::B8G8R8A8_UNorm;
        renderTargetDesc.Width = (UINT) (windowRect.right - windowRect.left);
        renderTargetDesc.Height = (UINT) (windowRect.bottom - windowRect.top);
        renderTargetDesc.SampleCount = 1;
        renderTargetDesc.MipLevels = 1;
        renderTargetDesc.Usage = ResourceUsage::Default;

        m_Backbuffers[i] = new DX12Texture(renderTarget, renderTargetDesc, m_Context);
    }

    m_CurrentBackbuffer = m_Swapchain->GetCurrentBackBufferIndex();
}

void DX12Swapchain::Present()
{
    m_Swapchain->Present(1, 0);
}

Texture* DX12Swapchain::GetCurrentBackBuffer()
{
    return m_Backbuffers[m_CurrentBackbuffer];
}

}