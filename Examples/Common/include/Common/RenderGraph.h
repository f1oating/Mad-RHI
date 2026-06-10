#pragma once

#include <cstdint>
#include "Mad-RHI/Device.h"
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

namespace mad::common {

class RenderGraph
{
struct Resource
{
    std::string Name;

    rhi::TextureFormat Format;
    uint32_t Width;
    uint32_t Height;
    uint8_t BindFlags;

    rhi::ResourceState InitalState;
    rhi::ResourceState FinalState;

    rhi::Texture* Texture;
};

struct Pass
{
    std::string Name;
    std::vector<std::string> Inputs;
    std::vector<std::string> Outputs;
    std::function<void(rhi::CommandQueue*)> ExecuteFn;
};

public:
    RenderGraph(rhi::Device* device) : m_Device(device) {}; 

    void AddResource(std::string name, rhi::TextureFormat format, uint32_t width, uint32_t height,
        uint8_t bindFlags, rhi::ResourceState initalState, rhi::ResourceState finalState);

    void AddPass(std::string name, std::vector<std::string> inputs,
        std::vector<std::string> outputs, std::function<void(rhi::CommandQueue*)> executeFn);

    void Compile();
    void Execute(rhi::CommandQueue* queue);

    Resource* GetResource(std::string name);

private:
    rhi::Device* m_Device;

    std::unordered_map<std::string, Resource> m_Resources;
    std::vector<Pass> m_Passes;
    std::vector<size_t> m_ExecutionOrder;

};

}