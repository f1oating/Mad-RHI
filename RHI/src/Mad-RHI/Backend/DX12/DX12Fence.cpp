#include "Mad-RHI/Backend/DX12/DX12Fence.h"
#include "Mad-RHI/Backend/DX12/DX12Device.h"

namespace mad::rhi {

DX12Fence::DX12Fence(DX12Device* context)
{
    m_Context = context;

    m_Context->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
    m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
}

DX12Fence::~DX12Fence()
{
    if (m_FenceEvent)
    {
        CloseHandle(m_FenceEvent);
    }
    if (m_Fence)
    {
        m_Fence->Release();
    }
}

uint64_t DX12Fence::GetCompletedValue()
{
    return m_Fence->GetCompletedValue();
}

uint64_t DX12Fence::GetCurrentValue()
{
    return m_CPUFenceValue;
}

uint64_t DX12Fence::IncrementCurrentValue()
{
    return ++m_CPUFenceValue;
}

void DX12Fence::Wait(uint64_t value)
{
    m_Fence->SetEventOnCompletion(value, m_FenceEvent);
    WaitForSingleObject(m_FenceEvent, INFINITE);
}

}