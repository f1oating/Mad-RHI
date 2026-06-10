#include "Common/RenderGraph.h"

namespace mad::common {

RGPassBuilder::RGPassBuilder(RenderGraph& rg, RGPass& pass)
    : m_RG(rg), m_Pass(pass)
{

}

rhi::Texture* RGPassBuilder::ReadTexture(RGTextureHandle handle)
{
    RGTextureEntry entry = m_RG.m_Textures[handle];

    m_Pass.Reads.push_back(entry);

    return entry.PhysicalTexture;
}

rhi::Texture* RGPassBuilder::WriteTexture(RGTextureHandle handle)
{
    RGTextureEntry entry = m_RG.m_Textures[handle];

    m_Pass.Writes.push_back(entry);

    return entry.PhysicalTexture;
}

RenderGraph::RenderGraph(rhi::Device* device) : m_Device(device)
{

}

void RenderGraph::AddPass(std::string name, std::function<void(RGPassBuilder&)> setup,
    std::function<void(RenderGraph&, rhi::CommandQueue*)> execute)
{
    RGPass pass;
    pass.Name = name;
    pass.Execute = execute;
    
    RGPassBuilder builder(*this, pass);
    setup(builder);

    m_Passes.push_back(pass);
}

void RenderGraph::Compile()
{
    for (RGTextureEntry entry : m_Textures)
    {
        if (!entry.IsImported) m_Device->CreateTexture(&entry.PhysicalTexture, entry.Desc);
    }
}

void RenderGraph::Execute(rhi::CommandQueue* queue)
{
    for (RGPass pass : m_Passes)
    {
        pass.Execute(*this, queue);
    }
}

RGTextureHandle RenderGraph::ImportTexture(rhi::Texture* texture)
{
    RGTextureHandle handle = m_Textures.size();

    m_Textures.push_back({ .Desc = texture->GetDesc(), .PhysicalTexture = texture, .IsImported = true });

    return handle;
}

RGTextureHandle RenderGraph::CreateTexture(const rhi::TextureDesc& desc)
{
    RGTextureHandle handle = m_Textures.size();

    m_Textures.push_back({ .Desc = desc });

    return handle;
}

}