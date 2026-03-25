#pragma once

namespace mad::rhi::vk {

class CommandListPool
{
public:
    CommandListPool() = default;
    ~CommandListPool() = default;

    void Init();
    void Shutdown();

};

}