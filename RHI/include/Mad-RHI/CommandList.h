#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/Resource.h"
#include <cstdint>
#include <vector>

namespace mad::rhi {

enum CommandQueueTypeFlags : uint32_t
{
    COMMAND_QUEUE_TYPE_GRAPHICS = 1 << 0,
    COMMAND_QUEUE_TYPE_COMPUTE = 1 << 1,
    COMMAND_QUEUE_TYPE_TRANSFER = 1 << 2,
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