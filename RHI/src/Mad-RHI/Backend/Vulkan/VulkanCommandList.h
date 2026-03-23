#pragma once

#include "Mad-RHI/CommandList.h"

namespace mad::rhi {

class VulkanGraphicsImmidiateCommandList : public ObjectBase<ImmidiateCommandList>
{
protected:
    ~VulkanGraphicsImmidiateCommandList();

public:
    VulkanGraphicsImmidiateCommandList();

};

}