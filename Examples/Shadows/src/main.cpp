#include <iostream>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Common/BootStrap.h"
#include "Common/Event.h"
#include "Common/Camera.h"
#include "Common/ShaderCompiler.h"
#include <chrono>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>

using namespace mad;
using namespace rhi;

struct Light
{
    glm::mat4 lightView;
    glm::mat4 lightProj;
};

struct Scene
{
    glm::mat4 world;
    glm::mat4 cameraView;
    glm::mat4 cameraProj;
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

        RefPtr<Buffer> lightBuffer = nullptr;
        RefPtr<Buffer> sceneBuffer = nullptr;
        RefPtr<Sampler> shadowSampler = nullptr;

        // Shadow pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/ShadowPassVertex.slang" });

            RefPtr<Shader> vertexShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());

            GraphicsPipelineDesc shadowPipelineDesc{};
            shadowPipelineDesc.VertexShader = vertexShader;
            shadowPipelineDesc.Topology = PrimitiveTopology::TriangleList;
            shadowPipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            shadowPipelineDesc.Rasterization.Cull = CullMode::Back;
            shadowPipelineDesc.Rasterization.Face = FrontFace::CW;
            shadowPipelineDesc.DepthStencil.DepthTestEnable = true;
            shadowPipelineDesc.DepthStencil.DepthWriteEnable = true;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            shadowPipelineDesc.BlendAttachments.push_back(colorBlend);
            shadowPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_Float;
            shadowPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(shadowPipeline.GetAddress(), shadowPipelineDesc);

            TextureDesc shadowMapDesc {};
            shadowMapDesc.Width = 2048;
            shadowMapDesc.Height = 2048;
            shadowMapDesc.Format = TextureFormat::D32_Float;
            shadowMapDesc.BindFlags = ResourceBind::RESOURCE_BIND_DEPTH_STENCIL | ResourceBind::RESOURCE_BIND_SHADER_RESOURSE;
            device->CreateTexture(shadowMap.GetAddress(), shadowMapDesc);
        }

        // Color pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/ColorPassVertex.slang" });
            std::vector<uint32_t> spirvPixel = common::ShaderCompiler::Compile({ "shaders/ColorPassPixel.slang" });

            RefPtr<Shader> vertexShader = nullptr;
            RefPtr<Shader> pixelShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
            device->CreateShader(pixelShader.GetAddress(), spirvPixel.data(), spirvPixel.size());

            GraphicsPipelineDesc colorPipelineDesc{};
            colorPipelineDesc.VertexShader = vertexShader;
            colorPipelineDesc.FragmentShader = pixelShader;
            colorPipelineDesc.Topology = PrimitiveTopology::TriangleList;
            colorPipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            colorPipelineDesc.Rasterization.Cull = CullMode::Front;
            colorPipelineDesc.Rasterization.Face = FrontFace::CW;
            colorPipelineDesc.DepthStencil.DepthTestEnable = true;
            colorPipelineDesc.DepthStencil.DepthWriteEnable = true;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            colorPipelineDesc.BlendAttachments.push_back(colorBlend);
            colorPipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::BGRA8_UNorm_SRGB);
            colorPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_Float;
            colorPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(colorPipeline.GetAddress(), colorPipelineDesc);

            TextureDesc colorDepthDesc {};
            colorDepthDesc.Width = 800;
            colorDepthDesc.Height = 600;
            colorDepthDesc.Format = TextureFormat::D32_Float;
            colorDepthDesc.BindFlags = ResourceBind::RESOURCE_BIND_DEPTH_STENCIL;
            device->CreateTexture(colorDepth.GetAddress(), colorDepthDesc);
        }

        // Resources
        {
            BufferDesc lightBufferDesc {};
            lightBufferDesc.Usage = ResourceUsage::Dynamic;
            lightBufferDesc.Size = sizeof(Light);
            lightBufferDesc.BindFlags = RESOURCE_BIND_UNIFORM_BUFFER;
            device->CreateBuffer(lightBuffer.GetAddress(), lightBufferDesc);

            BufferDesc sceneBufferDesc {};
            sceneBufferDesc.Usage = ResourceUsage::Dynamic;
            sceneBufferDesc.Size = sizeof(Scene);
            sceneBufferDesc.BindFlags = RESOURCE_BIND_UNIFORM_BUFFER;
            device->CreateBuffer(sceneBuffer.GetAddress(), sceneBufferDesc);

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
            camera.SetAspectRatio(event.Width / event.Height);
            
            swapchain->Resize();

            colorDepth.Reset();

            auto backbufferDesc = swapchain->GetCurrentBackBuffer()->GetDesc();

            TextureDesc colorDepthDesc {};
            colorDepthDesc.Width = backbufferDesc.Width;
            colorDepthDesc.Height = backbufferDesc.Height;
            colorDepthDesc.Format = TextureFormat::D32_Float;
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

                commandQueue->ClearDepthStencil(backBuffer->GetDefaultRTV().Get(), 1.0f, 0);
                
                commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

                void* lightData = lightBuffer->Map();
                Light light {};
                light.lightView = lightView;
                light.lightProj = lightProj;
                memcpy(lightData, &light, sizeof(Light));

                commandQueue->SetUniformBuffer("uLight", lightBuffer.Get());

                void* sceneData = sceneBuffer->Map();
                Scene scene {};
                scene.world = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
                scene.cameraView = camera.GetView();
                scene.cameraProj = camera.GetProjection();
                memcpy(sceneData, &scene, sizeof(Scene));

                commandQueue->SetUniformBuffer("uScene", sceneBuffer.Get());

                commandQueue->DrawIndexed(36, IndexType::Uint32);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                commandQueue->DrawIndexed(6, IndexType::Uint32);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                sceneData = sceneBuffer->Map();
                scene.world = glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 1.0f, 40.0f));
                memcpy(sceneData, &scene, sizeof(Scene));

                commandQueue->SetUniformBuffer("uScene", sceneBuffer.Get());

                commandQueue->DrawIndexed(6, IndexType::Uint32);

                commandQueue->ResourceBarrier({ {shadowMap.Get(), ResourceState::ShaderResource} }, {});
            }

            // Color pass
            {
                commandQueue->SetGraphicsPipeline(colorPipeline.Get());
                commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, colorDepth->GetDefaultDSV().Get());

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                commandQueue->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
                commandQueue->ClearDepthStencil(backBuffer->GetDefaultRTV().Get(), 1.0f, 0);
                
                commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

                void* lightData = lightBuffer->Map();
                Light light {};
                light.lightView = lightView;
                light.lightProj = lightProj;
                memcpy(lightData, &light, sizeof(Light));

                commandQueue->SetUniformBuffer("uLight", lightBuffer.Get());

                void* sceneData = sceneBuffer->Map();
                Scene scene {};
                scene.world = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
                scene.cameraView = camera.GetView();
                scene.cameraProj = camera.GetProjection();
                memcpy(sceneData, &scene, sizeof(Scene));

                commandQueue->SetUniformBuffer("uScene", sceneBuffer.Get());
                commandQueue->SetTexture("ShadowMap", shadowMap->GetDefaultSRV().Get());
                commandQueue->SetSampler("ShadowSampler", shadowSampler.Get());

                commandQueue->DrawIndexed(36, IndexType::Uint32);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                sceneData = sceneBuffer->Map();
                scene.world = glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 1.0f, 40.0f));
                memcpy(sceneData, &scene, sizeof(Scene));

                commandQueue->SetUniformBuffer("uScene", sceneBuffer.Get());

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