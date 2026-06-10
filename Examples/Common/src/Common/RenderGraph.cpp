#include "Common/RenderGraph.h"

namespace mad::common {

void RenderGraph::AddResource(std::string name, rhi::TextureFormat format, uint32_t width, uint32_t height,
    uint8_t bindFlags, rhi::ResourceState initalState, rhi::ResourceState finalState)
{
    Resource resource;
    resource.Name = name;
    resource.Format = format;
    resource.Width = width;
    resource.Height = height;
    resource.BindFlags = bindFlags;
    resource.InitalState = initalState;
    resource.FinalState = finalState;

    m_Resources[name] = resource;
}

void RenderGraph::AddPass(std::string name, std::vector<std::string> inputs,
    std::vector<std::string> outputs, std::function<void(rhi::CommandQueue*)> executeFn)
{
    Pass pass;
    pass.Name = name;
    pass.Inputs = inputs;
    pass.Outputs = outputs;
    pass.ExecuteFn = executeFn;

    m_Passes.push_back(pass);
}

void RenderGraph::Compile()
{
    std::vector<std::vector<size_t>> dependencies(m_Passes.size());
    std::vector<std::vector<size_t>> dependents(m_Passes.size());

    std::unordered_map<std::string, size_t> resourceWriters;

    for (size_t i = 0; i < m_Passes.size(); ++i)
    {
        const auto& pass = m_Passes[i];

        for (const auto& input : pass.Inputs) 
        {
            auto it = resourceWriters.find(input);
            if (it != resourceWriters.end()) 
            {
                dependencies[i].push_back(it->second);
                dependents[it->second].push_back(i);
            }
        }

        for (const auto& output : pass.Outputs) 
        {
            resourceWriters[output] = i;
        }
    }

    std::vector<bool> visited(m_Passes.size(), false);
    std::vector<bool> inStack(m_Passes.size(), false);

    std::function<void(size_t)> visit = [&](size_t node) 
    {
        if (inStack[node]) 
        {
            // Cycled dependency
        }

        if (visited[node]) 
        {
            return;
        }

        inStack[node] = true;

        for (auto dependent : dependents[node]) 
        {
            visit(dependent);
        }

        inStack[node] = false;
        visited[node] = true;
        m_ExecutionOrder.push_back(node);
    };

    for (size_t i = 0; i < m_Passes.size(); ++i) 
    {
        if (!visited[i]) 
        {
            visit(i);
        }
    }

    for (auto& [name, resource] : m_Resources) 
    {
        rhi::TextureDesc desc;
        desc.Format = resource.Format;
        desc.Width = resource.Width;
        desc.Height = resource.Height;
        desc.BindFlags = resource.BindFlags;

        m_Device->CreateTexture(&resource.Texture, desc);
    }
}

void RenderGraph::Execute(rhi::CommandQueue* queue)
{
    for (auto passIdx : m_ExecutionOrder) 
    {
        const auto& pass = m_Passes[passIdx];

        for (const auto& input : pass.Inputs) 
        {
            auto& resource = m_Resources[input];

            queue->ResourceBarrier({ {resource.Texture, rhi::ResourceState::ShaderResource} }, {});
        }

        for (const auto& output : pass.Outputs) 
        {
            auto& resource = m_Resources[output];

            queue->ResourceBarrier({ {resource.Texture, resource.InitalState} }, {});
        }

        pass.ExecuteFn(queue);

        for (const auto& output : pass.Outputs) 
        {
            auto& resource = m_Resources[output];

            queue->ResourceBarrier({ {resource.Texture, resource.FinalState} }, {});
        }
    }
}

RenderGraph::Resource* RenderGraph::GetResource(std::string name)
{
    return &m_Resources[name];
}

}