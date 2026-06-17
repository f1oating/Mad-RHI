#include "Mad-RHI/Backend/DX12/DX12Swapchain.h"
#include "Mad-RHI/Backend/DX12/DX12Device.h"

namespace mad::rhi {

DX12Swapchain::DX12Swapchain(WindowHandle window, DX12CommandQueue* queue, DX12Device* context)
{
    m_Context = context;
    m_Queue = queue;

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc {};
    swapchainDesc.Width = 800;
    swapchainDesc.Height = 600;
    swapchainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapchainDesc.Stereo = false;
    swapchainDesc.SampleDesc = { 1, 0 };
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = 3;
    swapchainDesc.Scaling = DXGI_SCALING_NONE;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = 0;

    m_Context->GetFactory()->GetFactory()->CreateSwapChainForHwnd(m_Context->GetDevice(), (HWND)window.win32.hwnd, 
        &swapchainDesc, nullptr, nullptr, &m_Swapchain);
}

DX12Swapchain::~DX12Swapchain()
{
    if (m_Swapchain)
    {
        m_Swapchain->Release();
    }
}

void DX12Swapchain::Resize()
{

}

void DX12Swapchain::Present()
{

}

Texture* DX12Swapchain::GetCurrentBackBuffer()
{

}

}