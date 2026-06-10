#pragma once

#include <cstdint>
#include "Mad-RHI/Device.h"
#include <functional>
#include <string>
#include <vector>

namespace mad::common {

using RGTextureHandle = uint16_t;

struct RGTextureEntry
{
    rhi::TextureDesc Desc;
    rhi::Texture* PhysicalTexture;

    bool IsImported = false;
};

class RenderGraph;

struct RGPass
{
    std::string Name;
    std::function<void(RenderGraph&, rhi::CommandQueue*)> Execute;
    std::vector<RGTextureEntry> Reads;
    std::vector<RGTextureEntry> Writes;
};

class RGPassBuilder;

class RenderGraph
{
friend class RGPassBuilder;

public:
    RenderGraph(rhi::Device* device);

    void AddPass(std::string name, std::function<void(RGPassBuilder&)> setup,
        std::function<void(RenderGraph&, rhi::CommandQueue*)> execute);

    void Compile();
    void Execute(rhi::CommandQueue* queue);

    RGTextureHandle ImportTexture(rhi::Texture* texture);

    RGTextureHandle CreateTexture(const rhi::TextureDesc& desc);

    rhi::Texture* GetTexture(RGTextureHandle handle) { return m_Textures[handle].PhysicalTexture; }

private:
    rhi::Device* m_Device = nullptr;

    std::vector<RGTextureEntry> m_Textures;
    std::vector<RGPass> m_Passes;

};

class RGPassBuilder
{
public:
    RGPassBuilder(RenderGraph& rg, RGPass& pass);

    rhi::Texture* ReadTexture(RGTextureHandle handle);
    rhi::Texture* WriteTexture(RGTextureHandle handle);

private:
    RenderGraph& m_RG;
    RGPass& m_Pass;

};

}