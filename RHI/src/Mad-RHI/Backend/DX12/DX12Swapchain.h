#pragma once

#include "Mad-RHI/Swapchain.h"
#include <directx/d3dx12.h>
#include <dxgi1_4.h>
#include "Mad-RHI/Backend/DX12/DX12Resource.h"
#include "Mad-RHI/Backend/DX12/DX12CommandQueue.h"
#include <vector>
#include "Mad-RHI/Backend/DX12/DX12Fence.h"

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
    WindowHandle m_Window;

    DX12Device* m_Context = nullptr;
    DX12CommandQueue* m_Queue = nullptr;

    IDXGISwapChain3* m_Swapchain = nullptr;

    uint32_t m_CurrentBackbuffer = 0;
    std::vector<DX12Texture*> m_Backbuffers;

    DX12Fence* m_Fence = nullptr;

};

}