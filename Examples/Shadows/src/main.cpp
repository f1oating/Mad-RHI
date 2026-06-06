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

using namespace mad;
using namespace rhi;

struct Transform
{
    glm::mat4 World;
    glm::mat4 View;
    glm::mat4 Proj;
};

struct Light
{
    glm::vec3 Dir;
    float __pad1;
};

int main()
{
    common::BootStrap bootStrap;
    bootStrap.Init("Shadows");

    {
        common::Window* window = bootStrap.GetWindow();
        Device* device = bootStrap.GetDevice();
        CommandQueue* commandQueue = bootStrap.GetQueue();
        Swapchain* swapchain = bootStrap.GetSwapchain();

        RefPtr<GraphicsPipelineState> shadowPipeline = nullptr;
        RefPtr<Texture> shadowMap = nullptr;

        RefPtr<GraphicsPipelineState> colorPipeline = nullptr;
        RefPtr<Texture> colorDepth = nullptr;

        RefPtr<Buffer> transformBuffer = nullptr;
        RefPtr<Buffer> lightBuffer = nullptr;
        RefPtr<Sampler> shadowSampler = nullptr;

        // Shadow pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderSystem::Compile({ "shaders/ShadowPassVertex.slang" });

            RefPtr<Shader> vertexShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());

            GraphicsPipelineDesc shadowPipelineDesc{};
            shadowPipelineDesc.VertexShader = vertexShader;
            shadowPipelineDesc.Topology = PrimitiveTopology::TriangleList;
            shadowPipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            shadowPipelineDesc.Rasterization.Cull = CullMode::Front;
            shadowPipelineDesc.Rasterization.Face = FrontFace::CCW;
            shadowPipelineDesc.DepthStencil.DepthTestEnable = true;
            shadowPipelineDesc.DepthStencil.DepthWriteEnable = true;
            shadowPipelineDesc.DepthStencil.DepthCompareOp = CompareOp::Less;
            shadowPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_SFloat;
            shadowPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(shadowPipeline.GetAddress(), shadowPipelineDesc);

            TextureDesc shadowMapDesc {};
            shadowMapDesc.Width = 2048;
            shadowMapDesc.Height = 2048;
            shadowMapDesc.Format = TextureFormat::D32_SFloat;
            shadowMapDesc.BindFlags = ResourceBind::RESOURCE_BIND_DEPTH_STENCIL | ResourceBind::RESOURCE_BIND_SHADER_RESOURSE;
            device->CreateTexture(shadowMap.GetAddress(), shadowMapDesc);
        }

        // Color pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderSystem::Compile({ "shaders/ColorPassVertex.slang" });
            std::vector<uint32_t> spirvPixel = common::ShaderSystem::Compile({ "shaders/ColorPassPixel.slang" });

            RefPtr<Shader> vertexShader = nullptr;
            RefPtr<Shader> pixelShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
            device->CreateShader(pixelShader.GetAddress(), spirvPixel.data(), spirvPixel.size());

            GraphicsPipelineDesc colorPipelineDesc{};
            colorPipelineDesc.VertexShader = vertexShader;
            colorPipelineDesc.FragmentShader = pixelShader;
            colorPipelineDesc.Topology = PrimitiveTopology::TriangleList;
            colorPipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            colorPipelineDesc.Rasterization.Cull = CullMode::Back;
            colorPipelineDesc.Rasterization.Face = FrontFace::CCW;
            colorPipelineDesc.DepthStencil.DepthTestEnable = true;
            colorPipelineDesc.DepthStencil.DepthWriteEnable = true;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            colorPipelineDesc.BlendAttachments.push_back(colorBlend);
            colorPipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::B8G8R8A8_SRGB_UNorm);
            colorPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_SFloat;
            colorPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(colorPipeline.GetAddress(), colorPipelineDesc);

            TextureDesc colorDepthDesc {};
            colorDepthDesc.Width = 800;
            colorDepthDesc.Height = 600;
            colorDepthDesc.Format = TextureFormat::D32_SFloat;
            colorDepthDesc.BindFlags = ResourceBind::RESOURCE_BIND_DEPTH_STENCIL;
            device->CreateTexture(colorDepth.GetAddress(), colorDepthDesc);
        }

        // Resources
        {
            BufferDesc transformBufferDesc {};
            transformBufferDesc.Usage = ResourceUsage::Dynamic;
            transformBufferDesc.Size = sizeof(Transform);
            transformBufferDesc.BindFlags = RESOURCE_BIND_UNIFORM_BUFFER;
            device->CreateBuffer(transformBuffer.GetAddress(), transformBufferDesc);

            BufferDesc lightBufferDesc {};
            lightBufferDesc.Usage = ResourceUsage::Dynamic;
            lightBufferDesc.Size = sizeof(Light);
            lightBufferDesc.BindFlags = RESOURCE_BIND_UNIFORM_BUFFER;
            device->CreateBuffer(lightBuffer.GetAddress(), lightBufferDesc);

            SamplerDesc shadowSamplerDesc {};
            shadowSamplerDesc.MinFilter = FilterType::Linear;
            shadowSamplerDesc.MagFilter = FilterType::Linear;
            shadowSamplerDesc.MipFilter = FilterType::Nearest;
            shadowSamplerDesc.AddressU = TextureAddressMode::ClampToBorder;
            shadowSamplerDesc.AddressV = TextureAddressMode::ClampToBorder;
            shadowSamplerDesc.Border = BorderColor::FloatOpaqueWhite;
            shadowSamplerDesc.Compare = CompareOp::LessEqual;
            device->CreateSampler(shadowSampler.GetAddress(), shadowSamplerDesc);
        }

        glm::vec3 lightDir = glm::normalize(glm::vec3(-0.3f, -0.3f, -1.0f));
        glm::vec3 lightPos = -lightDir * 50.0f;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), up);

        float size = 50.0f;
        float near = 0.1f;
        float far = 200.0f;

        glm::mat4 lightProj = glm::ortho(-size, size, -size, size, near, far);

        common::Camera camera { { 0.0f, 1.0f, 3.0f }, 90.0f, 800.0f / 600.0f, 0.1f, 100.0f };

        common::EventBus::Subscribe<common::WindowResizeEvent>([&colorDepth,
            &device, &swapchain, &camera](const common::WindowResizeEvent& event){
            swapchain->Resize();

            colorDepth.Reset();

            auto backbufferDesc = swapchain->GetCurrentBackBuffer()->GetDesc();
            camera.SetAspectRatio(static_cast<float>(backbufferDesc.Width) / static_cast<float>(backbufferDesc.Height));

            TextureDesc colorDepthDesc {};
            colorDepthDesc.Width = backbufferDesc.Width;
            colorDepthDesc.Height = backbufferDesc.Height;
            colorDepthDesc.Format = TextureFormat::D32_SFloat;
            colorDepthDesc.BindFlags = ResourceBind::RESOURCE_BIND_DEPTH_STENCIL;
            device->CreateTexture(colorDepth.GetAddress(), colorDepthDesc);
        });

        auto startTime = std::chrono::high_resolution_clock::now();

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float t = std::chrono::duration<float>(now - startTime).count();

            window->Update();

            // Camera move
            {
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
            }

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();

            commandQueue->ResourceBarrier({ {backBuffer, ResourceState::RenderTarget}, {shadowMap.Get(), ResourceState::DepthWrite} }, {});

            // Shadow pass
            {
                commandQueue->SetGraphicsPipeline(shadowPipeline.Get());
                commandQueue->SetRenderTargets({}, shadowMap->GetDefaultDSV().Get());

                commandQueue->ClearDepthStencil(colorDepth->GetDefaultDSV().Get(), 1.0f, 0);
                
                commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

                void* transformData = transformBuffer->Map();
                Transform transform {};
                transform.View = lightView;
                transform.Proj = lightProj;
                memcpy(transformData, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uLightTransform", transformBuffer.Get());

                transformData = transformBuffer->Map();
                transform.World = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
                transform.View = camera.GetView();
                transform.Proj = camera.GetProjection();
                memcpy(transformData, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uObjectTransform", transformBuffer.Get());

                commandQueue->DrawIndexed(36, IndexType::Uint32);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                transformData = transformBuffer->Map();
                transform.World = glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 1.0f, 40.0f));
                memcpy(transformData, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uObjectTransform", transformBuffer.Get());

                commandQueue->DrawIndexed(6, IndexType::Uint32);

                commandQueue->ResourceBarrier({ {shadowMap.Get(), ResourceState::ShaderResource} }, {});
            }

            // Color pass
            {
                commandQueue->SetGraphicsPipeline(colorPipeline.Get());
                commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, colorDepth->GetDefaultDSV().Get());

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                commandQueue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
                commandQueue->ClearDepthStencil(colorDepth->GetDefaultDSV().Get(), 1.0f, 0);
                
                commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

                void* lightData = lightBuffer->Map();
                Light light {};
                light.Dir = lightDir;
                memcpy(lightData, &light, sizeof(Light));

                commandQueue->SetUniformBuffer("uLight", lightBuffer.Get());

                void* transformData = transformBuffer->Map();
                Transform transform {};
                transform.View = lightView;
                transform.Proj = lightProj;
                memcpy(transformData, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uLightTransform", transformBuffer.Get());

                transformData = transformBuffer->Map();
                transform.World = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
                transform.View = camera.GetView();
                transform.Proj = camera.GetProjection();
                memcpy(transformData, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uObjectTransform", transformBuffer.Get());
                commandQueue->SetTexture("ShadowMap", shadowMap->GetDefaultSRV().Get());
                commandQueue->SetSampler("ShadowSampler", shadowSampler.Get());

                commandQueue->DrawIndexed(36, IndexType::Uint32);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                transformData = transformBuffer->Map();
                transform.World = glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 1.0f, 40.0f));
                memcpy(transformData, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uObjectTransform", transformBuffer.Get());

                commandQueue->DrawIndexed(6, IndexType::Uint32);
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