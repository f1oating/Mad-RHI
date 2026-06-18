#pragma once

#include "Mad-RHI/Swapchain.h"
#include <directx/d3dx12.h>
#include <dxgi1_2.h>
#include "Mad-RHI/Backend/DX12/DX12Resource.h"
#include "Mad-RHI/Backend/DX12/DX12CommandQueue.h"
#include <vector>

namespace mad::rhi {

class DX12Device;

class DX12Swapchain : public ObjectBase<Swapchain>
{
protected:
    ~DX12Swapchain();

public:
    DX12Swapchain(WindowHandle window, DX12CommandQueue* queue, DX12Device* context);

    virtual void Resize() override;

    virtual void Present() override;

    virtual Texture* GetCurrentBackBuffer() override;

private:
    DX12Device* m_Context = nullptr;
    DX12CommandQueue* m_Queue = nullptr;

    IDXGISwapChain1* m_Swapchain = nullptr;

    std::vector<DX12Texture*> m_Backbuffers;

};

}