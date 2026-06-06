#pragma once

#include <cstdint>

namespace mad::common {

struct RGTextureHandle
{
    uint16_t Index;
    uint16_t Version;
};

struct RGBufferHandle
{
    uint16_t Index;
    uint16_t Version;
};

class RenderGraph
{

};

}