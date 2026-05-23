#include <iostream>

#include "Common/BootStrap.h"
#include "Common/Event.h"
#include "Common/ShaderCompiler.h"
#include <chrono>
#include <cmath>

using namespace mad;
using namespace rhi;

int main()
{
    common::BootStrap bootStrap;
    bootStrap.Init("Triangle");

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
        pipelineDesc.Rasterization.Face = FrontFace::CW;
        pipelineDesc.DepthStencil.DepthTestEnable = false;
        pipelineDesc.DepthStencil.DepthWriteEnable = false;
        ColorAttachmentBlend colorBlend{};
        colorBlend.BlendEnable = false;
        pipelineDesc.BlendAttachments.push_back(colorBlend);
        pipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::BGRA8_UNorm_SRGB);
        pipelineDesc.Rendering.SampleCount = 1;
        RefPtr<GraphicsPipelineState> pipeline = nullptr;
        device->CreateGraphicsPipeline(pipeline.GetAddress(), pipelineDesc);

        BufferDesc cbd{};
        cbd.Usage = ResourceUsage::Dynamic;
        cbd.Size = 64;
        cbd.BindFlags = RESOURCE_BIND_UNIFORM_BUFFER;

        RefPtr<Buffer> cb = nullptr;
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
        RefPtr<Buffer> vb = nullptr;
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

            float* mapped = static_cast<float*>(cb->Map());
            mapped[0] = (std::sin(t * 1.0f) * 0.5f + 0.5f);
            mapped[1] = (std::sin(t * 1.5f) * 0.5f + 0.5f);
            mapped[2] = (std::sin(t * 2.0f) * 0.5f + 0.5f);
            mapped[3] = 1.0f;

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();

            commandQueue->ResourceBarrier({ {backBuffer, ResourceState::RenderTarget} }, {});

            commandQueue->SetGraphicsPipeline(pipeline.Get());
            commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, nullptr);

            float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
            commandQueue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
            
            commandQueue->SetVertexBuffers(0, { vb.Get() }, { 0 });
            commandQueue->SetUniformBuffer("uColor", cb.Get());
            commandQueue->Draw(3);

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