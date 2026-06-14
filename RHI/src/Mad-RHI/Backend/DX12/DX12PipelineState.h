#pragma once

#include "Mad-RHI/PipelineState.h"

namespace mad::rhi {

class DX12GraphicsPipelineState : public ObjectBase<GraphicsPipelineState>
{
protected:
    ~DX12GraphicsPipelineState();

public:
    DX12GraphicsPipelineState(const GraphicsPipelineDesc& desc);

private:
    

};

};