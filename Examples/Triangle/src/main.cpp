#include <iostream>

#include "Common/BootStrap.h"
#include "Common/Event.h"
#include "Common/ShaderCompiler.h"
#include <chrono>
#include <cmath>

using namespace mad;

int main()
{
    common::BootStrap bootStrap;
    bootStrap.Init("Triangle");

    {
        common::Window* window = bootStrap.GetWindow();
        rhi::Device* device = bootStrap.GetDevice();
        rhi::CommandQueue* commandQueue = bootStrap.GetQueue();
        rhi::Swapchain* swapchain = bootStrap.GetSwapchain();

        std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/Vertex.slang" });
        std::vector<uint32_t> spirvFragment = common::ShaderCompiler::Compile({ "shaders/Fragment.slang" });

        rhi::RefPtr<rhi::Shader> vertexShader = nullptr;
        rhi::RefPtr<rhi::Shader> fragmentShader = nullptr; 

        device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
        device->CreateShader(fragmentShader.GetAddress(), spirvFragment.data(), spirvFragment.size());

        rhi::GraphicsPipelineDesc pipelineDesc{};
        pipelineDesc.VertexShader = vertexShader;
        pipelineDesc.FragmentShader = fragmentShader;
        pipelineDesc.Topology = rhi::PrimitiveTopology::TriangleList;
        pipelineDesc.Rasterization.Polygon = rhi::PolygonMode::Fill;
        pipelineDesc.Rasterization.Cull = rhi::CullMode::Back;
        pipelineDesc.Rasterization.Face = rhi::FrontFace::CW;
        pipelineDesc.DepthStencil.DepthTestEnable = false;
        pipelineDesc.DepthStencil.DepthWriteEnable = false;
        rhi::ColorAttachmentBlend colorBlend{};
        colorBlend.BlendEnable = false;
        pipelineDesc.BlendAttachments.push_back(colorBlend);
        pipelineDesc.Rendering.ColorFormats.push_back(rhi::TextureFormat::BGRA8_UNorm_SRGB);
        pipelineDesc.Rendering.SampleCount = 1;
        rhi::RefPtr<rhi::GraphicsPipelineState> pipeline = nullptr;
        device->CreateGraphicsPipeline(pipeline.GetAddress(), pipelineDesc);

        rhi::BufferDesc cbd{};
        cbd.Usage = rhi::ResourceUsage::Dynamic;
        cbd.Size = 64;
        cbd.BindFlags = rhi::RESOURCE_BIND_UNIFORM_BUFFER;

        rhi::RefPtr<rhi::Buffer> cb = nullptr;
        device->CreateBuffer(cb.GetAddress(), cbd);
        
        float vertices[] = 
        {
            0.0f,  0.5f,  1.0f, 0.0f, 0.0f,
            0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.0f, 0.0f, 1.0f
        };

        rhi::BufferDesc vbd{};
        vbd.Usage = rhi::ResourceUsage::Default;
        vbd.Size = sizeof(vertices);
        vbd.BindFlags = rhi::RESOURCE_BIND_VERTEX_BUFFER;
        rhi::RefPtr<rhi::Buffer> vb = nullptr;
        device->CreateBuffer(vb.GetAddress(), vbd);

        commandQueue->ResourceBarrier({}, { {vb.Get(), rhi::ResourceState::CopyDst} });
        commandQueue->UpdateBuffer(vb.Get(), vertices, sizeof(vertices));
        commandQueue->ResourceBarrier({}, { {vb.Get(), rhi::ResourceState::VertexBuffer} });
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

            rhi::Texture* backBuffer = swapchain->GetCurrentBackBuffer();

            commandQueue->ResourceBarrier({ {backBuffer, rhi::ResourceState::RenderTarget} }, {});

            commandQueue->SetGraphicsPipeline(pipeline.Get());
            commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, nullptr);

            float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
            commandQueue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
            
            commandQueue->SetVertexBuffers(0, { vb.Get() }, { 0 });
            commandQueue->SetUniformBuffer("uColor", cb.Get());
            commandQueue->Draw(3);

            commandQueue->ResourceBarrier({ {backBuffer, rhi::ResourceState::Present} }, {});

            commandQueue->Flush();

            device->EndFrame();
            swapchain->Present();
        }
    }

    common::EventBus::Clear();
    bootStrap.Shutdown();

    return 0;
}