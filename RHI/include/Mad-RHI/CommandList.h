#pragma once

#include "Mad-RHI/Common.h"

namespace mad::rhi {

class DefferedCommandList : public Object
{
public:
    virtual ~DefferedCommandList() = default;

};

class ImmidiateCommandList : public Object
{
public:
    virtual ~ImmidiateCommandList() = default;

    virtual void Flush() = 0;

};

}