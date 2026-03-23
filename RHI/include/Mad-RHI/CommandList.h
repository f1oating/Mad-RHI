#pragma once

#include "Mad-RHI/Common.h"

namespace mad::rhi {
    
class CommandList : Object
{
public:
    virtual ~CommandList() = default;

};

class DefferedCommandList : public CommandList
{
public:
    virtual ~DefferedCommandList() = default;

};

class ImmidiateCommandList : public DefferedCommandList
{
public:
    virtual ~ImmidiateCommandList() = default;

};

}