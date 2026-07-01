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

#include <windows.h>
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 619; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

int main()
{
    Factory::Init({ FactoryBackend::DX12, "PBR", "Mad-RHI" });

    common::Window* window = new common::Window("PBR", 800, 600);
    Factory* factory = Factory::Get();
    Device* device = nullptr;
    CommandQueue* queue = nullptr;
    Swapchain* swapchain = nullptr;

    rhi::CommandQueueDesc queueDesc{};
    queueDesc.Type = rhi::CommandQueueType::COMMAND_QUEUE_TYPE_GRAPHICS;

    rhi::DeviceDesc deviceDesc{};
    deviceDesc.AdapterId = 0;
    deviceDesc.pCommandQueues = &queueDesc;
    deviceDesc.NumCommandQueues = 1;

    factory->CreateDevice(&device, deviceDesc);

    queue = device->GetCommandQueue(0);
    device->CreateSwapchain(&swapchain, window->GetWindowInfo(), queue);

    TextureDesc texDesc {};
    texDesc.Width = 4;
    texDesc.Height = 4;

    Texture* tex = nullptr;
    device->CreateTexture(&tex, texDesc);

    BufferDesc buffDesc {};
    buffDesc.Size = 4;
    buffDesc.BindFlags = RESOURCE_BIND_UNIFORM_BUFFER;

    Buffer* buff = nullptr;
    device->CreateBuffer(&buff, buffDesc);

    common::EventBus::Subscribe<common::WindowResizeEvent>([&swapchain](const common::WindowResizeEvent& event){
        swapchain->Resize();
    });

    {
        auto startTime = std::chrono::high_resolution_clock::now();
        auto prevTime = startTime;

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - prevTime).count();
            prevTime = now;

            window->Update();
            common::ShaderSystem::Poll();

            Texture* backbuffer = swapchain->GetCurrentBackBuffer();

            queue->ResourceBarrier( { {backbuffer, ResourceState::RenderTarget} }, {} );

            queue->ResourceBarrier( { {backbuffer, ResourceState::Present} }, {} );

            swapchain->Present();
        }
    }

    common::EventBus::Clear();

    buff->Release();
    tex->Release();

    swapchain->Release();
    device->Release();

    Factory::Shutdown();

    return 0;
}