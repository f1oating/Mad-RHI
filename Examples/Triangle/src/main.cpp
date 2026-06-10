#include <iostream>

#include "Common/BootStrap.h"
#include "Common/Event.h"
#include "Common/ShaderSystem.h"
#include <chrono>
#include <cmath>
#include "Common/RenderGraph.h"

using namespace mad;
using namespace rhi;
using namespace common;

int main()
{
    common::BootStrap bootStrap;
    bootStrap.Init("Triangle");

    {
        common::Window* window = bootStrap.GetWindow();
        Device* device = bootStrap.GetDevice();
        CommandQueue* commandQueue = bootStrap.GetQueue();
        Swapchain* swapchain = bootStrap.GetSwapchain();

        RefPtr<GraphicsPipelineState> pipeline = nullptr;
        RefPtr<Buffer> cb = nullptr;
        RefPtr<Buffer> vb = nullptr;

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
            pipelineDesc.Rasterization.Face = FrontFace::CW;
            pipelineDesc.DepthStencil.DepthTestEnable = false;
            pipelineDesc.DepthStencil.DepthWriteEnable = false;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            pipelineDesc.BlendAttachments.push_back(colorBlend);
            pipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::B8G8R8A8_SRGB_UNorm);
            pipelineDesc.Rendering.SampleCount = 1;
            
            device->CreateGraphicsPipeline(pipeline.GetAddress(), pipelineDesc);
        };

        pipelineCreateCallback();
        common::ShaderSystem::WatchShader({ "shaders/Vertex.slang", "shaders/Fragment.slang" }, pipelineCreateCallback);

        BufferDesc cbd{};
        cbd.Usage = ResourceUsage::Dynamic;
        cbd.Size = 64;
        cbd.BindFlags = RESOURCE_BIND_UNIFORM_BUFFER;

        device->CreateBuffer(cb.GetAddress(), cbd);
        
        float vertices[] = 
        {
            0.0f,  0.5f,  1.0f, 0.0f, 0.0f,
            0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.0f, 0.0f, 1.0f
        };

        BufferDesc vbd{};
        vbd.Usage = ResourceUsage::Default;
        vbd.Size = sizeof(vertices);
        vbd.BindFlags = RESOURCE_BIND_VERTEX_BUFFER;
        
        device->CreateBuffer(vb.GetAddress(), vbd);

        commandQueue->ResourceBarrier({}, { {vb.Get(), ResourceState::CopyDst} });
        commandQueue->UpdateBuffer(vb.Get(), vertices, sizeof(vertices));
        commandQueue->ResourceBarrier({}, { {vb.Get(), ResourceState::VertexBuffer} });
        commandQueue->Flush();

        auto startTime = std::chrono::high_resolution_clock::now();

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float t = std::chrono::duration<float>(now - startTime).count();

            window->Update();
            common::ShaderSystem::Poll();

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();

            common::RenderGraph renderGraph(device);

            renderGraph.AddPass("Color", {}, {}, [&t, &cb, &vb, &pipeline, &backBuffer](CommandQueue* queue){
                float* mapped = static_cast<float*>(cb->Map());
                mapped[0] = (std::sin(t * 1.0f) * 0.5f + 0.5f);
                mapped[1] = (std::sin(t * 1.5f) * 0.5f + 0.5f);
                mapped[2] = (std::sin(t * 2.0f) * 0.5f + 0.5f);
                mapped[3] = 1.0f;

                queue->ResourceBarrier({ {backBuffer, ResourceState::RenderTarget} }, {});

                queue->SetGraphicsPipeline(pipeline.Get());
                queue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, nullptr);

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                queue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
                
                queue->SetVertexBuffers(0, { vb.Get() }, { 0 });
                queue->SetUniformBuffer("uColor", cb.Get());
                queue->Draw(3);

                queue->ResourceBarrier({ {backBuffer, ResourceState::Present} }, {});
            });

            renderGraph.Compile();
            renderGraph.Execute(commandQueue);

            commandQueue->Flush();

            device->EndFrame();
            swapchain->Present();
        }
    }

    common::EventBus::Clear();
    bootStrap.Shutdown();

    return 0;
}