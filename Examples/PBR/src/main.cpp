#include <iostream>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Common/BootStrap.h"
#include "Common/Event.h"
#include "Common/Camera.h"
#include "Common/ShaderSystem.h"
#include <chrono>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>
#include "Common/RenderGraph.h"

using namespace mad;
using namespace rhi;

int main()
{
    Factory::Init({ FactoryBackend::DX12, "PBR", "Mad-RHI" });

    Factory* factory = Factory::Get();
    Device* device = nullptr;
    CommandQueue* queue = nullptr;

    rhi::CommandQueueDesc queueDesc{};
    queueDesc.Type = rhi::CommandQueueType::COMMAND_QUEUE_TYPE_GRAPHICS;

    rhi::DeviceDesc deviceDesc{};
    deviceDesc.AdapterId = 0;
    deviceDesc.pCommandQueues = &queueDesc;
    deviceDesc.NumCommandQueues = 1;

    factory->CreateDevice(&device, deviceDesc);

    queue = device->GetCommandQueue(0);

    TextureDesc desc {};
    desc.Width = 4;
    desc.Height = 4;

    Texture* tex = nullptr;
    device->CreateTexture(&tex, desc);

    {
        common::Window* window = new common::Window("PBR", 800, 600);

        auto startTime = std::chrono::high_resolution_clock::now();
        auto prevTime = startTime;

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - prevTime).count();
            prevTime = now;

            window->Update();
            common::ShaderSystem::Poll();
        }
    }

    common::EventBus::Clear();

    tex->Release();

    device->Release();

    Factory::Shutdown();

    return 0;
}