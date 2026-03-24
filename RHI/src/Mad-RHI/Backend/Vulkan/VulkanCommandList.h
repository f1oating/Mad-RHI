#pragma once

#include "Mad-RHI/CommandList.h"

namespace mad::rhi {

class VulkanImmidiateCommandList : public ObjectBase<ImmidiateCommandList>
{
protected:
    ~VulkanImmidiateCommandList();

public:
    VulkanImmidiateCommandList();

};

}