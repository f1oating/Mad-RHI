#include "Common/RenderGraph.h"

namespace mad::common {

RGPassBuilder::RGPassBuilder(RenderGraph& rg, RGPass& pass)
    : m_RG(rg), m_Pass(pass)
{

}

rhi::TextureView* RGPassBuilder::GetTextureSRV(RGTextureHandle handle)
{
    RGTextureEntry entry = m_RG.m_Textures[handle];

    entry.ComputedBindFlags |= rhi::RESOURCE_BIND_SHADER_RESOURSE;
    m_Pass.Reads.push_back(entry);

    return entry.PhysicalTexture->GetDefaultSRV().Get();
}

rhi::TextureView* RGPassBuilder::GetTextureRTV(RGTextureHandle handle)
{
    RGTextureEntry entry = m_RG.m_Textures[handle];

    entry.ComputedBindFlags |= rhi::RESOURCE_BIND_RENDER_TARGET;
    m_Pass.Writes.push_back(entry);

    return entry.PhysicalTexture->GetDefaultRTV().Get();
}

rhi::TextureView* RGPassBuilder::GetTextureDSV(RGTextureHandle handle)
{
    RGTextureEntry entry = m_RG.m_Textures[handle];

    entry.ComputedBindFlags |= rhi::RESOURCE_BIND_DEPTH_STENCIL;
    m_Pass.Writes.push_back(entry);

    return entry.PhysicalTexture->GetDefaultDSV().Get();
}

void RenderGraph::AddPass(std::string name, std::function<void(rhi::CommandQueue*)> execute,
    std::function<void(RGPassBuilder&)> setup)
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
        rhi::TextureDesc desc;
        desc.Dimension = entry.Desc.Dimension;
        desc.Width = entry.Desc.Width;
        desc.Height = entry.Desc.Height;
        desc.ArraySize = entry.Desc.ArraySize;
        desc.Depth = entry.Desc.Depth;
        desc.Format = entry.Desc.Format;
        desc.MipLevels = entry.Desc.MipLevels;
        desc.SampleCount = entry.Desc.SampleCount;
        desc.BindFlags = entry.ComputedBindFlags;

        m_Device->CreateTexture(entry.PhysicalTexture.GetAddress(), desc);
    }
}

void RenderGraph::Execute(rhi::CommandQueue* queue)
{
    for (RGPass pass : m_Passes)
    {
        pass.Execute(queue);
    }
}

RGTextureHandle RenderGraph::CreateTexture(RGTextureDesc desc)
{
    RGTextureHandle handle = m_Textures.size();

    m_Textures.push_back({ .Desc = desc });

    return handle;
}

}