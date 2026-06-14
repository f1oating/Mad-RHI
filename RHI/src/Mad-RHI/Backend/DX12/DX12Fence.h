#pragma once

#include "Mad-RHI/Fence.h"

namespace mad::rhi {

class DX12Fence : public ObjectBase<Fence>
{
protected:
    ~DX12Fence();

public:
    DX12Fence();

    virtual uint64_t GetCompletedValue() override;
    virtual uint64_t GetCurrentValue() override;
    
    virtual uint64_t IncrementCurrentValue() override;

    virtual void Wait(uint64_t value) override;
    
private:

};

}