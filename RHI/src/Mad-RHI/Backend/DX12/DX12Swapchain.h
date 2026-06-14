#pragma once

#include "Mad-RHI/Swapchain.h"

namespace mad::rhi {

class DX12Swapchain : public ObjectBase<Swapchain>
{
protected:
    ~DX12Swapchain();

public:
    DX12Swapchain();

    virtual void Resize() override;

    virtual void Present() override;

    virtual Texture* GetCurrentBackBuffer() override;

private:

};

}