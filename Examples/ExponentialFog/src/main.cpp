#include <iostream>

#include "Common/BootStrap.h"
#include "Common/Event.h"
#include "Common/ShaderCompiler.h"
#include "Common/Camera.h"
#include <chrono>
#include <cmath>
#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

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

        std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/Vertex.slang" });
        std::vector<uint32_t> spirvFragment = common::ShaderCompiler::Compile({ "shaders/Fragment.slang" });

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
        pipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::BGRA8_UNorm_SRGB);
        pipelineDesc.Rendering.DepthFormat = TextureFormat::D32_Float;
        pipelineDesc.Rendering.SampleCount = 1;
        RefPtr<GraphicsPipelineState> pipeline = nullptr;
        device->CreateGraphicsPipeline(pipeline.GetAddress(), pipelineDesc);

        struct Transform
        {
            glm::mat4 Model;
            glm::mat4 View;
            glm::mat4 Proj;
        };

        RefPtr<Buffer> cb = nullptr;
        BufferDesc cbd{};
        cbd.BindFlags = ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
        cbd.Size = sizeof(Transform);
        cbd.Usage = ResourceUsage::Dynamic;
        device->CreateBuffer(cb.GetAddress(), cbd);

        RefPtr<Texture> depthBuffer = nullptr;
        TextureDesc depthBufferDesc {};
        depthBufferDesc.BindFlags = ResourceBind::RESOURCE_BIND_DEPTH_STENCIL;
        depthBufferDesc.Width = 800;
        depthBufferDesc.Height = 600;
        depthBufferDesc.Format = TextureFormat::D32_Float;
        device->CreateTexture(depthBuffer.GetAddress(), depthBufferDesc);

        common::Camera camera { { 0.0f, 1.0f, 3.0f }, 90.0f, 800.0f / 600.0f, 0.1f, 100.0f };

        common::EventBus::Subscribe<common::WindowResizeEvent>([&depthBuffer, &device, &swapchain, &camera](const common::WindowResizeEvent& event){
            swapchain->Resize();

            depthBuffer.Reset();

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();
            const TextureDesc& backBufferDesc = backBuffer->GetDesc();

            TextureDesc depthBufferDesc {};
            depthBufferDesc.BindFlags = ResourceBind::RESOURCE_BIND_DEPTH_STENCIL;
            depthBufferDesc.Width = backBufferDesc.Width;
            depthBufferDesc.Height = backBufferDesc.Height;
            depthBufferDesc.Format = TextureFormat::D32_Float;
            device->CreateTexture(depthBuffer.GetAddress(), depthBufferDesc);

            camera.SetAspectRatio(static_cast<float>(backBufferDesc.Width) / static_cast<float>(backBufferDesc.Height));
        });

        auto startTime = std::chrono::high_resolution_clock::now();

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float t = std::chrono::duration<float>(now - startTime).count();

            window->Update();

            const bool* keys = SDL_GetKeyboardState(nullptr);
            float dx, dy;
            SDL_GetRelativeMouseState(&dx, &dy);

            camera.Rotate(dx, dy);

            if (keys[SDL_SCANCODE_W])
            {
                camera.MoveForward(t);
            }
            if (keys[SDL_SCANCODE_S])
            {
                camera.MoveBack(t);
            }
            if (keys[SDL_SCANCODE_A])
            {
                camera.MoveLeft(t);
            }
            if (keys[SDL_SCANCODE_D])
            {
                camera.MoveRight(t);
            }
            if (keys[SDL_SCANCODE_ESCAPE])
            {
                static bool mode = false;
                mode = !mode;
                window->SetRelativeMode(mode);
            }

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();

            commandQueue->ResourceBarrier({ {backBuffer, ResourceState::RenderTarget}, {depthBuffer.Get(), ResourceState::DepthWrite} }, {});

            commandQueue->SetGraphicsPipeline(pipeline.Get());
            commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, depthBuffer->GetDefaultDSV().Get());

            float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
            commandQueue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
            commandQueue->ClearDepthStencil(depthBuffer->GetDefaultDSV().Get(), 1.0f, 0);

            commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
            commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

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

                    commandQueue->SetUniformBuffer("uTransform", cb.Get());

                    commandQueue->DrawIndexed(36, IndexType::Uint32);
                }
            }

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