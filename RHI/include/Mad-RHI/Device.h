#pragma once

#include "Mad-RHI/Common.h"
#include "Mad-RHI/CommandList.h"
#include "Mad-RHI/PipelineState.h"

namespace mad::rhi {
    
class Device : public Object
{
public:
    virtual ~Device() = default;

    virtual void ReleaseStaleResources() = 0;
    virtual void Resize() = 0;

    virtual void Present() = 0;

    virtual RefPtr<ImmidiateCommandList> GetImmidiateCommandList() = 0;

    virtual RefPtr<Shader> CreateShader(const uint32_t* data, uint64_t size) = 0;
    virtual RefPtr<GraphicsPipelineState> CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;

};

}