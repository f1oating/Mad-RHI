#pragma once

#include "Mad-RHI/Fence.h"
#include <directx/d3d12.h>

namespace mad::rhi {

class DX12Device;

class DX12Fence : public ObjectBase<Fence>
{
protected:
    ~DX12Fence();

public:
    DX12Fence(DX12Device* context);

    virtual uint64_t GetCompletedValue() override;
    virtual uint64_t GetCurrentValue() override;
    
    virtual uint64_t IncrementCurrentValue() override;

    virtual void Wait(uint64_t value) override;
    
    ID3D12Fence* GetFence() { return m_Fence; }

private:
    DX12Device* m_Context = nullptr;

    ID3D12Fence* m_Fence = nullptr;
    HANDLE m_FenceEvent = nullptr;
    uint64_t m_CPUFenceValue = 0;

};

}