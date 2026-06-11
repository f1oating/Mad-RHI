#include <iostream>

#include "Common/BootStrap.h"
#include "Common/Event.h"
#include "Common/ShaderSystem.h"
#include "Common/Camera.h"
#include <chrono>
#include <cmath>
#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Common/RenderGraph.h"

using namespace mad;
using namespace rhi;

int main()
{
    common::BootStrap bootStrap;
    bootStrap.Init("Exponential Fog");

    {
        common::Window* window = bootStrap.GetWindow();
        Device* device = bootStrap.GetDevice();
        CommandQueue* commandQueue = bootStrap.GetQueue();
        Swapchain* swapchain = bootStrap.GetSwapchain();

        RefPtr<GraphicsPipelineState> pipeline = nullptr;
        RefPtr<Buffer> cb = nullptr;

        std::function<void()> pipelineCreateCallback = [&device, &pipeline](){
            pipeline.Reset();

            std::vector<uint32_t> spirvVertex = common::ShaderSystem::Compile({ "shaders/Vertex.slang" });
            std::vector<uint32_t> spirvFragment = common::ShaderSystem::Compile({ "shaders/Fragment.slang" });

            RefPtr<Shader> vertexShader = nullptr;
            RefPtr<Shader> fragmentShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
            device->CreateShader(fragmentShader.GetAddress(), spirvFragment.data(), spirvFragment.size());

            GraphicsPipelineDesc pipelineDesc{};
            pipelineDesc.VertexShader = vertexShader;
            pipelineDesc.FragmentShader = fragmentShader;
            pipelineDesc.Topology = PrimitiveTopology::TriangleList;
            pipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            pipelineDesc.Rasterization.Cull = CullMode::Back;
            pipelineDesc.Rasterization.Face = FrontFace::CCW;
            pipelineDesc.DepthStencil.DepthTestEnable = true;
            pipelineDesc.DepthStencil.DepthWriteEnable = true;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            pipelineDesc.BlendAttachments.push_back(colorBlend);
            pipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::B8G8R8A8_SRGB_UNorm);
            pipelineDesc.Rendering.DepthFormat = TextureFormat::D32_SFloat;
            pipelineDesc.Rendering.SampleCount = 1;
            
            device->CreateGraphicsPipeline(pipeline.GetAddress(), pipelineDesc);
        };

        pipelineCreateCallback();
        common::ShaderSystem::WatchShader({ "shaders/Vertex.slang", "shaders/Fragment.slang" }, pipelineCreateCallback);

        struct Transform
        {
            glm::mat4 Model;
            glm::mat4 View;
            glm::mat4 Proj;
        };

        BufferDesc cbd{};
        cbd.BindFlags = ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
        cbd.Size = sizeof(Transform);
        cbd.Usage = ResourceUsage::Dynamic;
        device->CreateBuffer(cb.GetAddress(), cbd);

        common::Camera camera { { 0.0f, 1.0f, 3.0f }, 90.0f, 800.0f / 600.0f, 0.1f, 100.0f };

        common::EventBus::Subscribe<common::WindowResizeEvent>([&device, &swapchain, &camera](const common::WindowResizeEvent& event){
            swapchain->Resize();

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();
            const TextureDesc& backBufferDesc = backBuffer->GetDesc();

            camera.SetAspectRatio(static_cast<float>(backBufferDesc.Width) / static_cast<float>(backBufferDesc.Height));
        });

        auto startTime = std::chrono::high_resolution_clock::now();
        auto prevTime = startTime;

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - prevTime).count();
            prevTime = now;

            window->Update();
            common::ShaderSystem::Poll();

            const bool* keys = SDL_GetKeyboardState(nullptr);
            float dx, dy;
            SDL_GetRelativeMouseState(&dx, &dy);

            camera.Rotate(dx, dy);

            if (keys[SDL_SCANCODE_W])
            {
                camera.MoveForward(dt);
            }
            if (keys[SDL_SCANCODE_S])
            {
                camera.MoveBack(dt);
            }
            if (keys[SDL_SCANCODE_A])
            {
                camera.MoveLeft(dt);
            }
            if (keys[SDL_SCANCODE_D])
            {
                camera.MoveRight(dt);
            }
            if (keys[SDL_SCANCODE_ESCAPE])
            {
                static bool mode = false;
                mode = !mode;
                window->SetRelativeMode(mode);
            }

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();
            auto backbufferDesc = swapchain->GetCurrentBackBuffer()->GetDesc();

            commandQueue->ResourceBarrier({ {backBuffer, ResourceState::RenderTarget} }, {});

            common::RenderGraph renderGraph(device);

            renderGraph.AddResource("Depth", TextureFormat::D32_SFloat, backbufferDesc.Width, backbufferDesc.Height,
                RESOURCE_BIND_DEPTH_STENCIL, ResourceState::DepthWrite, ResourceState::DepthWrite);

            renderGraph.AddPass("Color", {}, { "Depth" }, [&cb, &camera, &pipeline, &bootStrap, 
                &backBuffer, &renderGraph](CommandQueue* queue){
                Texture* depth = renderGraph.GetResource("Depth")->Texture;

                queue->SetGraphicsPipeline(pipeline.Get());
                queue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, depth->GetDefaultDSV().Get());

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                queue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
                queue->ClearDepthStencil(depth->GetDefaultDSV().Get(), 1.0f, 0);

                queue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                queue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

                for (int col = 0; col < 3; col++)
                {
                    for (int row = 0; row < 3; row++)
                    {
                        Transform transform;
                        transform.Model = glm::translate(glm::mat4(1.0f), glm::vec3(col * 2.5f, 0.0f, row * 2.5f));
                        transform.View = camera.GetView();
                        transform.Proj = camera.GetProjection();

                        void* data = cb->Map();
                        memcpy(data, &transform, sizeof(Transform));

                        queue->SetUniformBuffer("uTransform", cb.Get());

                        queue->DrawIndexed(36, IndexType::Uint32);
                    }
                }
            });

            renderGraph.Compile();
            renderGraph.Execute(commandQueue);

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