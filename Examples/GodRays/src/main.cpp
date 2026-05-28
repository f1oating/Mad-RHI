#include <iostream>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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

struct Scene
{
    glm::mat4 InvViewProj;
    glm::vec3 CameraPos;
    float _pad0;
    glm::mat4 LightViewProj;
    glm::vec3 LightDir;
    float _pad1;
};

struct Light
{
    glm::mat4 LightViewProj;
    glm::vec3 LightDir;
    float _pad1;
};

int main()
{
    common::BootStrap bootStrap;
    bootStrap.Init("GodRays");

    {
        common::Window* window = bootStrap.GetWindow();
        Device* device = bootStrap.GetDevice();
        CommandQueue* commandQueue = bootStrap.GetQueue();
        Swapchain* swapchain = bootStrap.GetSwapchain();

        RefPtr<Sampler> shadowMapSampler = nullptr;
        RefPtr<Sampler> sceneDepthSampler = nullptr;

        RefPtr<Buffer> transformConstantBuffer = nullptr;
        RefPtr<Buffer> sceneConstantBuffer = nullptr;
        RefPtr<Buffer> lightConstantBuffer = nullptr;

        RefPtr<GraphicsPipelineState> shadowMapPipeline = nullptr;
        RefPtr<Texture> shadowMapTexture = nullptr;

        RefPtr<GraphicsPipelineState> colorPipeline = nullptr;
        RefPtr<Texture> colorTexture = nullptr;
        RefPtr<Texture> depthTexture = nullptr;

        RefPtr<GraphicsPipelineState> godRaysPipeline = nullptr;
        RefPtr<Texture> godRaysTexture = nullptr;

        RefPtr<GraphicsPipelineState> compositePipeline = nullptr;
        RefPtr<Sampler> compositeSampler = nullptr;

        // Resources
        {
            SamplerDesc shadowMapSamplerDesc {};
            shadowMapSamplerDesc.MinFilter = FilterType::Linear;
            shadowMapSamplerDesc.MagFilter = FilterType::Linear;
            shadowMapSamplerDesc.MipFilter = FilterType::Nearest;
            shadowMapSamplerDesc.AddressU = TextureAddressMode::ClampToBorder;
            shadowMapSamplerDesc.AddressV = TextureAddressMode::ClampToBorder;
            shadowMapSamplerDesc.Border = BorderColor::FloatOpaqueWhite;
            shadowMapSamplerDesc.Compare = CompareOp::LessEqual;
            device->CreateSampler(shadowMapSampler.GetAddress(), shadowMapSamplerDesc);

            SamplerDesc sceneDepthSamplerDesc {};
            sceneDepthSamplerDesc.MinFilter = FilterType::Nearest;
            sceneDepthSamplerDesc.MagFilter = FilterType::Nearest;
            sceneDepthSamplerDesc.MipFilter = FilterType::Nearest;
            sceneDepthSamplerDesc.AddressU = TextureAddressMode::ClampToEdge;
            sceneDepthSamplerDesc.AddressV = TextureAddressMode::ClampToEdge;
            device->CreateSampler(sceneDepthSampler.GetAddress(), sceneDepthSamplerDesc);

            BufferDesc transformConstantBufferDesc {};
            transformConstantBufferDesc.BindFlags = ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
            transformConstantBufferDesc.Size = sizeof(Transform);
            transformConstantBufferDesc.Usage = ResourceUsage::Dynamic;
            device->CreateBuffer(transformConstantBuffer.GetAddress(), transformConstantBufferDesc);

            BufferDesc sceneConstantBufferDesc {};
            sceneConstantBufferDesc.BindFlags = ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
            sceneConstantBufferDesc.Size = sizeof(Scene);
            sceneConstantBufferDesc.Usage = ResourceUsage::Dynamic;
            device->CreateBuffer(sceneConstantBuffer.GetAddress(), sceneConstantBufferDesc);

            BufferDesc lightConstantBufferDesc {};
            lightConstantBufferDesc.BindFlags = ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
            lightConstantBufferDesc.Size = sizeof(Light);
            lightConstantBufferDesc.Usage = ResourceUsage::Dynamic;
            device->CreateBuffer(lightConstantBuffer.GetAddress(), lightConstantBufferDesc);
        }

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
            shadowMapPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_Float;
            shadowMapPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(shadowMapPipeline.GetAddress(), shadowMapPipelineDesc);

            TextureDesc shadowMapDesc {};
            shadowMapDesc.Width = 2048;
            shadowMapDesc.Height = 2048;
            shadowMapDesc.BindFlags = RESOURCE_BIND_DEPTH_STENCIL | RESOURCE_BIND_SHADER_RESOURSE;
            shadowMapDesc.Format = TextureFormat::D32_Float;
            device->CreateTexture(shadowMapTexture.GetAddress(), shadowMapDesc);
        }

        // Color pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/ColorVertex.slang" });
            std::vector<uint32_t> spirvPixel = common::ShaderCompiler::Compile({ "shaders/ColorPixel.slang" });

            RefPtr<Shader> vertexShader = nullptr;
            RefPtr<Shader> pixelShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
            device->CreateShader(pixelShader.GetAddress(), spirvPixel.data(), spirvPixel.size());

            GraphicsPipelineDesc colorPipelineDesc {};
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
            colorPipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::BGRA8_UNorm_SRGB);
            colorPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_Float;
            colorPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(colorPipeline.GetAddress(), colorPipelineDesc);
            
            TextureDesc colorTextureDesc {};
            colorTextureDesc.Width = 800;
            colorTextureDesc.Height = 600;
            colorTextureDesc.BindFlags = RESOURCE_BIND_RENDER_TARGET | RESOURCE_BIND_SHADER_RESOURSE;
            colorTextureDesc.Format = TextureFormat::BGRA8_UNorm_SRGB;
            device->CreateTexture(colorTexture.GetAddress(), colorTextureDesc);

            TextureDesc depthTextureDesc {};
            depthTextureDesc.Width = 800;
            depthTextureDesc.Height = 600;
            depthTextureDesc.BindFlags = RESOURCE_BIND_DEPTH_STENCIL | RESOURCE_BIND_SHADER_RESOURSE;
            depthTextureDesc.Format = TextureFormat::D32_Float;
            device->CreateTexture(depthTexture.GetAddress(), depthTextureDesc);
        }

        // GodRays pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/GodRaysVertex.slang" });
            std::vector<uint32_t> spirvPixel = common::ShaderCompiler::Compile({ "shaders/GodRaysPixel.slang" });
            
            RefPtr<Shader> vertexShader = nullptr;
            RefPtr<Shader> pixelShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
            device->CreateShader(pixelShader.GetAddress(), spirvPixel.data(), spirvPixel.size());

            GraphicsPipelineDesc godRaysPipelineDesc {};
            godRaysPipelineDesc.VertexShader = vertexShader;
            godRaysPipelineDesc.FragmentShader = pixelShader;
            godRaysPipelineDesc.Topology = PrimitiveTopology::TriangleList;
            godRaysPipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            godRaysPipelineDesc.Rasterization.Cull = CullMode::None;
            godRaysPipelineDesc.Rasterization.Face = FrontFace::CCW;
            godRaysPipelineDesc.DepthStencil.DepthTestEnable = false;
            godRaysPipelineDesc.DepthStencil.DepthWriteEnable = false;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            godRaysPipelineDesc.BlendAttachments.push_back(colorBlend);
            godRaysPipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::RGBA16_Float);
            godRaysPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(godRaysPipeline.GetAddress(), godRaysPipelineDesc);

            TextureDesc godRaysTextureDesc {};
            godRaysTextureDesc.Width = 400;
            godRaysTextureDesc.Height = 300;
            godRaysTextureDesc.BindFlags = RESOURCE_BIND_RENDER_TARGET | RESOURCE_BIND_SHADER_RESOURSE;
            godRaysTextureDesc.Format = TextureFormat::RGBA16_Float;
            device->CreateTexture(godRaysTexture.GetAddress(), godRaysTextureDesc);
        }

        // Composite pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/CompositeVertex.slang" });
            std::vector<uint32_t> spirvPixel = common::ShaderCompiler::Compile({ "shaders/CompositePixel.slang" });

            RefPtr<Shader> vertexShader = nullptr;
            RefPtr<Shader> pixelShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
            device->CreateShader(pixelShader.GetAddress(), spirvPixel.data(), spirvPixel.size());

            GraphicsPipelineDesc compositePipelineDesc {};
            compositePipelineDesc.VertexShader = vertexShader;
            compositePipelineDesc.FragmentShader = pixelShader;
            compositePipelineDesc.Topology = PrimitiveTopology::TriangleList;
            compositePipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            compositePipelineDesc.Rasterization.Cull = CullMode::None;
            compositePipelineDesc.Rasterization.Face = FrontFace::CCW;
            compositePipelineDesc.DepthStencil.DepthTestEnable = false;
            compositePipelineDesc.DepthStencil.DepthWriteEnable = false;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            compositePipelineDesc.BlendAttachments.push_back(colorBlend);
            compositePipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::BGRA8_UNorm_SRGB);
            compositePipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(compositePipeline.GetAddress(), compositePipelineDesc);

            SamplerDesc compositeSamplerDesc {};
            device->CreateSampler(compositeSampler.GetAddress(), compositeSamplerDesc);
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

        common::EventBus::Subscribe<common::WindowResizeEvent>([&depthTexture, &colorTexture,
            &godRaysTexture, &device, &swapchain, &camera](const common::WindowResizeEvent& event){
            swapchain->Resize();

            colorTexture.Reset();
            depthTexture.Reset();
            godRaysTexture.Reset();

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();
            const TextureDesc& backBufferDesc = backBuffer->GetDesc();

            TextureDesc colorTextureDesc {};
            colorTextureDesc.Width = backBufferDesc.Width;
            colorTextureDesc.Height = backBufferDesc.Height;
            colorTextureDesc.BindFlags = RESOURCE_BIND_RENDER_TARGET | RESOURCE_BIND_SHADER_RESOURSE;
            colorTextureDesc.Format = TextureFormat::BGRA8_UNorm_SRGB;
            device->CreateTexture(colorTexture.GetAddress(), colorTextureDesc);

            TextureDesc depthTextureDesc {};
            depthTextureDesc.Width = backBufferDesc.Width;
            depthTextureDesc.Height = backBufferDesc.Height;
            depthTextureDesc.BindFlags = RESOURCE_BIND_DEPTH_STENCIL | RESOURCE_BIND_SHADER_RESOURSE;
            depthTextureDesc.Format = TextureFormat::D32_Float;
            device->CreateTexture(depthTexture.GetAddress(), depthTextureDesc);

            TextureDesc godRaysTextureDesc {};
            godRaysTextureDesc.Width = backBufferDesc.Width / 2;
            godRaysTextureDesc.Height = backBufferDesc.Height / 2;
            godRaysTextureDesc.BindFlags = RESOURCE_BIND_RENDER_TARGET | RESOURCE_BIND_SHADER_RESOURSE;
            godRaysTextureDesc.Format = TextureFormat::RGBA16_Float;
            device->CreateTexture(godRaysTexture.GetAddress(), godRaysTextureDesc);

            camera.SetAspectRatio(static_cast<float>(backBufferDesc.Width) / static_cast<float>(backBufferDesc.Height));
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

            commandQueue->ResourceBarrier({ 
                {backBuffer, ResourceState::RenderTarget}, 
                {colorTexture.Get(), ResourceState::RenderTarget},
                {shadowMapTexture.Get(), ResourceState::DepthWrite},
                {depthTexture.Get(), ResourceState::DepthWrite},
                {godRaysTexture.Get(), ResourceState::RenderTarget},
            }, {});

            // Shadow map pass
            {
                commandQueue->SetGraphicsPipeline(shadowMapPipeline.Get());
                commandQueue->SetRenderTargets({ }, shadowMapTexture->GetDefaultDSV().Get());

                commandQueue->ClearDepthStencil(shadowMapTexture->GetDefaultDSV().Get(), 1.0f, 0);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

                Transform transform;

                for (int col = 0; col < 3; col++)
                {
                    for (int row = 0; row < 3; row++)
                    {
                        transform.Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f + col * 2, 1.5f + row * 2, 0.0f));
                        transform.View = lightView;
                        transform.Proj = lightProj;

                        void* data = transformConstantBuffer->Map();
                        memcpy(data, &transform, sizeof(Transform));

                        commandQueue->SetUniformBuffer("uTransform", transformConstantBuffer.Get());

                        commandQueue->DrawIndexed(36, IndexType::Uint32);
                    }
                }

                commandQueue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                transform.Model = glm::scale(glm::mat4(1), {20, 1, 20});
                transform.View = lightView;
                transform.Proj = lightProj;

                void* data = transformConstantBuffer->Map();
                memcpy(data, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uTransform", transformConstantBuffer.Get());

                commandQueue->DrawIndexed(6, IndexType::Uint32);

                commandQueue->ResourceBarrier({ {shadowMapTexture.Get(), ResourceState::ShaderResource} }, {});
            }
            
            // Color pass
            {
                commandQueue->SetGraphicsPipeline(colorPipeline.Get());
                commandQueue->SetRenderTargets({ colorTexture->GetDefaultRTV().Get() }, depthTexture->GetDefaultDSV().Get());

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                commandQueue->ClearRenderTarget(colorTexture->GetDefaultRTV().Get(), clearColor);
                commandQueue->ClearDepthStencil(depthTexture->GetDefaultDSV().Get(), 1.0f, 0);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

                Light light;
                light.LightDir = lightDir;
                light.LightViewProj = lightProj * lightView;

                void* lightData = lightConstantBuffer->Map();
                memcpy(lightData, &light, sizeof(Light));

                commandQueue->SetTexture("ShadowMap", shadowMapTexture->GetDefaultDSV().Get());
                commandQueue->SetSampler("ShadowSamp", shadowMapSampler.Get());
                commandQueue->SetUniformBuffer("uLight", lightConstantBuffer.Get());

                Transform transform;

                for (int col = 0; col < 3; col++)
                {
                    for (int row = 0; row < 3; row++)
                    {
                        transform.Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f + col * 2, 1.5f + row * 2, 0.0f));
                        transform.View = camera.GetView();
                        transform.Proj = camera.GetProjection();

                        void* data = transformConstantBuffer->Map();
                        memcpy(data, &transform, sizeof(Transform));

                        commandQueue->SetUniformBuffer("uTransform", transformConstantBuffer.Get());

                        commandQueue->DrawIndexed(36, IndexType::Uint32);
                    }
                }

                commandQueue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                transform.Model = glm::scale(glm::mat4(1), {40, 1, 40});
                transform.View = camera.GetView();
                transform.Proj = camera.GetProjection();

                void* data = transformConstantBuffer->Map();
                memcpy(data, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uTransform", transformConstantBuffer.Get());

                commandQueue->DrawIndexed(6, IndexType::Uint32);

                commandQueue->ResourceBarrier({ {colorTexture.Get(), ResourceState::ShaderResource}, {depthTexture.Get(), ResourceState::ShaderResource} }, {});
            }

            // GodRays pass
            {
                commandQueue->SetGraphicsPipeline(godRaysPipeline.Get());
                commandQueue->SetRenderTargets({ godRaysTexture->GetDefaultRTV().Get() }, nullptr);

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                commandQueue->ClearRenderTarget(godRaysTexture->GetDefaultRTV().Get(), clearColor);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetFullScreenQuadVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetFullScreenQuadIndexBuffer());

                commandQueue->SetSampler("PointClamp", sceneDepthSampler.Get());
                commandQueue->SetSampler("ShadowSamp", shadowMapSampler.Get());
                commandQueue->SetTexture("SceneDepth", depthTexture->GetDefaultSRV().Get());
                commandQueue->SetTexture("ShadowMap", shadowMapTexture->GetDefaultSRV().Get());

                Scene scene;
                scene.CameraPos = camera.GetPosition();
                scene.LightDir = lightDir;
                scene.LightViewProj = lightProj * lightView;
                scene.InvViewProj = glm::inverse(camera.GetProjection() * camera.GetView());

                void* data = sceneConstantBuffer->Map();
                memcpy(data, &scene, sizeof(Scene));

                commandQueue->SetUniformBuffer("uScene", sceneConstantBuffer.Get());

                commandQueue->DrawIndexed(6, IndexType::Uint32);

                commandQueue->ResourceBarrier({ {godRaysTexture.Get(), ResourceState::ShaderResource} }, {});
            }

            // Composite pass
            {
                commandQueue->SetGraphicsPipeline(compositePipeline.Get());
                commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, nullptr);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetFullScreenQuadVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetFullScreenQuadIndexBuffer());

                commandQueue->SetTexture("SceneColor", colorTexture->GetDefaultSRV().Get());
                commandQueue->SetTexture("GodRays", godRaysTexture->GetDefaultSRV().Get());
                commandQueue->SetSampler("LinearClamp", compositeSampler.Get());

                commandQueue->DrawIndexed(6, IndexType::Uint32);
            }

            commandQueue->ResourceBarrier({ 
                {backBuffer, rhi::ResourceState::Present},
                {shadowMapTexture.Get(), ResourceState::ShaderResource}
            }, {});

            commandQueue->Flush();

            device->EndFrame();
            swapchain->Present();
        }
    }

    common::EventBus::Clear();
    bootStrap.Shutdown();

    return 0;
}