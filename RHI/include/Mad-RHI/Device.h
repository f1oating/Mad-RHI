#pragma once

#include "Mad-RHI/Common.h"

namespace mad::rhi {
    
class Device : public Object
{
public:
    virtual ~Device() = default;

    virtual void Resize() = 0;

    virtual void Present() = 0;

};

}