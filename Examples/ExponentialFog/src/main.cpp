#include <iostream>

#include "Common/BootStrap.h"
#include "Common/Event.h"
#include "Common/ShaderCompiler.h"
#include "Common/Camera.h"
#include <chrono>
#include <cmath>

using namespace mad;

int main()
{
    common::BootStrap bootStrap;
    bootStrap.Init("Exponential Fog");

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
        pipelineDesc.DepthStencil.DepthTestEnable = true;
        pipelineDesc.DepthStencil.DepthWriteEnable = true;
        rhi::ColorAttachmentBlend colorBlend{};
        colorBlend.BlendEnable = false;
        pipelineDesc.BlendAttachments.push_back(colorBlend);
        pipelineDesc.Rendering.ColorFormats.push_back(rhi::TextureFormat::BGRA8_UNorm_SRGB);
        pipelineDesc.Rendering.DepthFormat = rhi::TextureFormat::D32_Float;
        pipelineDesc.Rendering.SampleCount = 1;
        rhi::RefPtr<rhi::GraphicsPipelineState> pipeline = nullptr;
        device->CreateGraphicsPipeline(pipeline.GetAddress(), pipelineDesc);

        struct Transform
        {
            glm::mat4 Model;
            glm::mat4 View;
            glm::mat4 Proj;
        };

        rhi::RefPtr<rhi::Buffer> cb = nullptr;
        rhi::BufferDesc cbd{};
        cbd.BindFlags = rhi::ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
        cbd.Size = sizeof(Transform);
        cbd.Usage = rhi::ResourceUsage::Dynamic;
        device->CreateBuffer(cb.GetAddress(), cbd);

        common::Camera camera { { 0.0f, 1.0f, 3.0f }, 90.0f, 800.0f / 600.0f, 0.1f, 100.0f };

        Transform transform;
        transform.Model = glm::mat4(1.0f);
        transform.View = camera.GetView();
        transform.Proj = camera.GetProjection();

        auto startTime = std::chrono::high_resolution_clock::now();

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float t = std::chrono::duration<float>(now - startTime).count();

            window->Update();

            void* data = cb->Map();
            memcpy(data, &transform, sizeof(Transform));

            rhi::Texture* backBuffer = swapchain->GetCurrentBackBuffer();
            rhi::Texture* depthTexture = swapchain->GetCurrentDepthTexture();

            commandQueue->ResourceBarrier({ {backBuffer, rhi::ResourceState::RenderTarget}, {depthTexture, rhi::ResourceState::DepthWrite} }, {});

            commandQueue->SetGraphicsPipeline(pipeline.Get());
            commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, depthTexture->GetDefaultDSV().Get());

            float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
            commandQueue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
            commandQueue->ClearDepthStencil(depthTexture->GetDefaultDSV().Get(), 1.0f, 0);

            commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
            commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

            commandQueue->SetUniformBuffer("uTransform", cb.Get());

            commandQueue->DrawIndexed(36, rhi::IndexType::Uint32);

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