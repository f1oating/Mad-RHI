#pragma once

#include <cstdint>
#include "Mad-RHI/Device.h"
#include <functional>
#include <string>
#include <vector>

namespace mad::common {

using RGTextureHandle = uint16_t;

struct RGTextureDesc
{
    rhi::TextureDimension Dimension = rhi::TextureDimension::Texture2D;
    uint32_t Width = 0;
    uint32_t Height = 0;
    union 
    {
        uint32_t ArraySize = 1;
        uint32_t Depth; 
    };
    rhi::TextureFormat Format = rhi::TextureFormat::R8G8B8A8_UNorm;
    uint32_t MipLevels = 1;
    uint32_t SampleCount = 1;
};

struct RGTextureEntry
{
    RGTextureDesc Desc;
    uint8_t ComputedBindFlags;
    rhi::RefPtr<rhi::Texture> PhysicalTexture;
};

struct RGPass
{
    std::string Name;
    std::function<void(rhi::CommandQueue*)> Execute;
    std::vector<RGTextureEntry> Reads;
    std::vector<RGTextureEntry> Writes;
};

class RGPassBuilder;

class RenderGraph
{
friend class RGPassBuilder;

public:
    RenderGraph(rhi::Device* device);

    void AddPass(std::string name, std::function<void(rhi::CommandQueue*)> execute,
        std::function<void(RGPassBuilder&)> setup);

    void Compile();
    void Execute(rhi::CommandQueue* queue);

    RGTextureHandle CreateTexture(RGTextureDesc desc);

private:
    rhi::Device* m_Device = nullptr;

    std::vector<RGTextureEntry> m_Textures;
    std::vector<RGPass> m_Passes;

};

class RGPassBuilder
{
public:
    RGPassBuilder(RenderGraph& rg, RGPass& pass);

    rhi::TextureView* GetTextureSRV(RGTextureHandle handle);
    rhi::TextureView* GetTextureRTV(RGTextureHandle handle);
    rhi::TextureView* GetTextureDSV(RGTextureHandle handle);

private:
    RenderGraph m_RG;
    RGPass m_Pass;

};

}