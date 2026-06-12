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
    common::BootStrap bootStrap;
    bootStrap.Init("PBR");

    {
        common::Window* window = bootStrap.GetWindow();
        Device* device = bootStrap.GetDevice();
        CommandQueue* commandQueue = bootStrap.GetQueue();
        Swapchain* swapchain = bootStrap.GetSwapchain();

        auto startTime = std::chrono::high_resolution_clock::now();
        auto prevTime = startTime;

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - prevTime).count();
            prevTime = now;

            window->Update();
            common::ShaderSystem::Poll();

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();
            auto backbufferDesc = swapchain->GetCurrentBackBuffer()->GetDesc();

            commandQueue->ResourceBarrier({ {backBuffer, ResourceState::Present} }, {});

            commandQueue->Flush();

            device->EndFrame();
            swapchain->Present();
        }
    }

    common::EventBus::Clear();
    bootStrap.Shutdown();

    return 0;
}