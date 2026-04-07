#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/Resource.h"
#include <cstdint>
#include <vector>

namespace mad::rhi {

enum ImmidiateCommandListFlags : uint32_t
{
    Graphics = 1 << 0
};

class ImmidiateCommandList : public Object
{
public:
    virtual ~ImmidiateCommandList() = default;

    virtual void ResourceBarrier(
        std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers) = 0;

    virtual void Flush() = 0;

};

}