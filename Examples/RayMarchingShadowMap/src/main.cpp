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

struct Transform
{
    glm::mat4 Model;
    glm::mat4 View;
    glm::mat4 Proj;
};

int main()
{
    common::BootStrap bootStrap;
    bootStrap.Init("Ray Marching Shadow Map");

    {
        common::Window* window = bootStrap.GetWindow();
        Device* device = bootStrap.GetDevice();
        CommandQueue* commandQueue = bootStrap.GetQueue();
        Swapchain* swapchain = bootStrap.GetSwapchain();

        RefPtr<GraphicsPipelineState> shadowMapPipeline = nullptr;
        RefPtr<Texture> shadowMapTexture = nullptr;
        RefPtr<Buffer> shadowMapConstantBuffer = nullptr;

        // Shadow map pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/ShadowMapVertex.slang" });
            RefPtr<Shader> vertexShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());

            GraphicsPipelineDesc shadowMapPipelineDesc {};
            shadowMapPipelineDesc.VertexShader = vertexShader;
            shadowMapPipelineDesc.Topology = PrimitiveTopology::TriangleList;
            shadowMapPipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            shadowMapPipelineDesc.Rasterization.Cull = CullMode::Front;
            shadowMapPipelineDesc.Rasterization.Face = FrontFace::CCW;
            shadowMapPipelineDesc.DepthStencil.DepthTestEnable = true;
            shadowMapPipelineDesc.DepthStencil.DepthWriteEnable = true;
            shadowMapPipelineDesc.DepthStencil.DepthCompareOp = CompareOp::Less;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            shadowMapPipelineDesc.BlendAttachments.push_back(colorBlend);
            shadowMapPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_Float;
            shadowMapPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(shadowMapPipeline.GetAddress(), shadowMapPipelineDesc);

            TextureDesc shadowMapDesc {};
            shadowMapDesc.Width = 2048;
            shadowMapDesc.Height = 2048;
            shadowMapDesc.BindFlags = RESOURCE_BIND_DEPTH_STENCIL | RESOURCE_BIND_SHADER_RESOURSE;
            shadowMapDesc.Format = TextureFormat::D32_Float;
            device->CreateTexture(shadowMapTexture.GetAddress(), shadowMapDesc);
            
            BufferDesc shadowMapConstantBufferDesc {};
            shadowMapConstantBufferDesc.BindFlags = ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
            shadowMapConstantBufferDesc.Size = sizeof(Transform);
            shadowMapConstantBufferDesc.Usage = ResourceUsage::Dynamic;
            device->CreateBuffer(shadowMapConstantBuffer.GetAddress(), shadowMapConstantBufferDesc);
        }

        glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
        glm::vec3 lightPos = -lightDir * 50.0f;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), up);

        float size = 50.0f;
        float near = 0.1f;
        float far = 200.0f;

        glm::mat4 lightProj = glm::ortho(-size, size, -size, size, near, far);

        auto startTime = std::chrono::high_resolution_clock::now();

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float t = std::chrono::duration<float>(now - startTime).count();

            window->Update();

            rhi::Texture* backBuffer = swapchain->GetCurrentBackBuffer();

            commandQueue->ResourceBarrier({ {backBuffer, rhi::ResourceState::RenderTarget}, {shadowMapTexture.Get(), ResourceState::DepthWrite} }, {});

            commandQueue->SetGraphicsPipeline(shadowMapPipeline.Get());
            commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, shadowMapTexture->GetDefaultDSV().Get());

            float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
            commandQueue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
            commandQueue->ClearDepthStencil(shadowMapTexture->GetDefaultDSV().Get(), 1.0f, 0);

            commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
            commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

            Transform transform;
            transform.Model = glm::mat4(1.0f);
            transform.View = lightView;
            transform.Proj = lightProj;

            void* data = shadowMapConstantBuffer->Map();
            memcpy(data, &transform, sizeof(Transform));

            commandQueue->SetUniformBuffer("uTransform", shadowMapConstantBuffer.Get());

            commandQueue->DrawIndexed(36, IndexType::Uint32);

            commandQueue->ResourceBarrier({ {backBuffer, rhi::ResourceState::Present}, {shadowMapTexture.Get(), ResourceState::ShaderResource} }, {});

            commandQueue->Flush();

            device->EndFrame();
            swapchain->Present();
        }
    }

    common::EventBus::Clear();
    bootStrap.Shutdown();

    return 0;
}