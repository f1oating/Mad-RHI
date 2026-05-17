#pragma once

#include "Mad-RHI/Common.h"
#include <cstdint>

namespace mad::rhi {
    
class Fence : public Object
{
public:
    virtual ~Fence() = default;

    virtual uint64_t GetCompletedValue() = 0;
    virtual uint64_t GetCurrentValue() = 0;
    
    virtual uint64_t IncrementCurrentValue() = 0;

    virtual void Wait(uint64_t value) = 0;

};

}