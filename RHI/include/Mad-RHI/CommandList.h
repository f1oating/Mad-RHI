#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/Resource.h"
#include <cstdint>
#include <vector>

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

    virtual void ResourceBarrier(
        std::vector<TextureBarrier> textureBarriers, std::vector<BufferBarrier> bufferBarriers) = 0;

    virtual void Flush() = 0;

};

}