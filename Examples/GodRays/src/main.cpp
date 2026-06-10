#include <iostream>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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

        RefPtr<GraphicsPipelineState> colorPipeline = nullptr;

        RefPtr<GraphicsPipelineState> godRaysPipeline = nullptr;

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
            std::function<void()> pipelineCreateCallback = [&device, &shadowMapPipeline](){
                shadowMapPipeline.Reset();

                std::vector<uint32_t> spirvVertex = common::ShaderSystem::Compile({ "shaders/ShadowMapVertex.slang" });
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
                shadowMapPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_SFloat;
                shadowMapPipelineDesc.Rendering.SampleCount = 1;
                
                device->CreateGraphicsPipeline(shadowMapPipeline.GetAddress(), shadowMapPipelineDesc);
            };

            pipelineCreateCallback();
            common::ShaderSystem::WatchShader({ "shaders/ShadowMapVertex.slang" }, pipelineCreateCallback);
        }

        // Color pass
        {
            std::function<void()> pipelineCreateCallback = [&device, &colorPipeline](){
                colorPipeline.Reset();

                std::vector<uint32_t> spirvVertex = common::ShaderSystem::Compile({ "shaders/ColorVertex.slang" });
                std::vector<uint32_t> spirvPixel = common::ShaderSystem::Compile({ "shaders/ColorPixel.slang" });

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
                colorPipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::B8G8R8A8_SRGB_UNorm);
                colorPipelineDesc.Rendering.DepthFormat = TextureFormat::D32_SFloat;
                colorPipelineDesc.Rendering.SampleCount = 1;
                
                device->CreateGraphicsPipeline(colorPipeline.GetAddress(), colorPipelineDesc);
            };

            pipelineCreateCallback();
            common::ShaderSystem::WatchShader({ "shaders/ColorVertex.slang", "shaders/ColorPixel.slang" }, pipelineCreateCallback);
        }

        // GodRays pass
        {
            std::function<void()> pipelineCreateCallback = [&device, &godRaysPipeline](){
                godRaysPipeline.Reset();

                std::vector<uint32_t> spirvVertex = common::ShaderSystem::Compile({ "shaders/GodRaysVertex.slang" });
                std::vector<uint32_t> spirvPixel = common::ShaderSystem::Compile({ "shaders/GodRaysPixel.slang" });
                
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
                godRaysPipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::R16G16B16A16_SFloat);
                godRaysPipelineDesc.Rendering.SampleCount = 1;
                
                device->CreateGraphicsPipeline(godRaysPipeline.GetAddress(), godRaysPipelineDesc);
            };

            pipelineCreateCallback();
            common::ShaderSystem::WatchShader({ "shaders/GodRaysVertex.slang", "shaders/GodRaysPixel.slang" }, pipelineCreateCallback);
        }

        // Composite pass
        {
            std::function<void()> pipelineCreateCallback = [&device, &compositePipeline](){
                compositePipeline.Reset();

                std::vector<uint32_t> spirvVertex = common::ShaderSystem::Compile({ "shaders/CompositeVertex.slang" });
                std::vector<uint32_t> spirvPixel = common::ShaderSystem::Compile({ "shaders/CompositePixel.slang" });

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
                compositePipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::B8G8R8A8_SRGB_UNorm);
                compositePipelineDesc.Rendering.SampleCount = 1;
                
                device->CreateGraphicsPipeline(compositePipeline.GetAddress(), compositePipelineDesc);
            };

            pipelineCreateCallback();
            common::ShaderSystem::WatchShader({ "shaders/CompositeVertex.slang", "shaders/CompositePixel.slang" }, pipelineCreateCallback);

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

        common::EventBus::Subscribe<common::WindowResizeEvent>([&device, &swapchain, &camera](const common::WindowResizeEvent& event){
            swapchain->Resize();

            Texture* backBuffer = swapchain->GetCurrentBackBuffer();
            const TextureDesc& backBufferDesc = backBuffer->GetDesc();

            camera.SetAspectRatio(static_cast<float>(backBufferDesc.Width) / static_cast<float>(backBufferDesc.Height));
        });

        auto startTime = std::chrono::high_resolution_clock::now();

        while (window->IsRunning())
        {
            auto now = std::chrono::high_resolution_clock::now();
            float t = std::chrono::duration<float>(now - startTime).count();

            window->Update();
            common::ShaderSystem::Poll();

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
            }, {});

            auto backbufferDesc = swapchain->GetCurrentBackBuffer()->GetDesc();

            common::RenderGraph renderGraph(device);

            renderGraph.AddResource("ShadowMap", TextureFormat::D32_SFloat, 2048, 2048, 
                ResourceBind::RESOURCE_BIND_DEPTH_STENCIL | ResourceBind::RESOURCE_BIND_SHADER_RESOURSE,
                ResourceState::DepthWrite, ResourceState::ShaderResource);

            renderGraph.AddPass("Shadow", {}, { "ShadowMap" }, [&shadowMapPipeline, &bootStrap,
                &lightView, &lightProj, &transformConstantBuffer, &renderGraph](CommandQueue* queue){
                Texture* shadowMap = renderGraph.GetResource("ShadowMap")->Texture;

                queue->SetGraphicsPipeline(shadowMapPipeline.Get());
                queue->SetRenderTargets({ }, shadowMap->GetDefaultDSV().Get());

                queue->ClearDepthStencil(shadowMap->GetDefaultDSV().Get(), 1.0f, 0);

                queue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                queue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

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

                        queue->SetUniformBuffer("uTransform", transformConstantBuffer.Get());

                        queue->DrawIndexed(36, IndexType::Uint32);
                    }
                }

                queue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                queue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                transform.Model = glm::scale(glm::mat4(1), {20, 1, 20});
                transform.View = lightView;
                transform.Proj = lightProj;

                void* data = transformConstantBuffer->Map();
                memcpy(data, &transform, sizeof(Transform));

                queue->SetUniformBuffer("uTransform", transformConstantBuffer.Get());

                queue->DrawIndexed(6, IndexType::Uint32);
            });

            renderGraph.AddResource("Depth", TextureFormat::D32_SFloat, backbufferDesc.Width, backbufferDesc.Height, 
                ResourceBind::RESOURCE_BIND_DEPTH_STENCIL | ResourceBind::RESOURCE_BIND_SHADER_RESOURSE,
                ResourceState::DepthWrite, ResourceState::ShaderResource);

            renderGraph.AddResource("Color", TextureFormat::B8G8R8A8_SRGB_UNorm, backbufferDesc.Width, backbufferDesc.Height, 
                ResourceBind::RESOURCE_BIND_RENDER_TARGET | ResourceBind::RESOURCE_BIND_SHADER_RESOURSE,
                ResourceState::RenderTarget, ResourceState::ShaderResource);

            renderGraph.AddPass("Color", { "ShadowMap" }, { "Color", "Depth" }, [&colorPipeline, &bootStrap,
                &lightDir, &lightView, &lightProj, &transformConstantBuffer, &lightConstantBuffer, &shadowMapSampler,
                &camera, &renderGraph](CommandQueue* queue){
                Texture* shadowMap = renderGraph.GetResource("ShadowMap")->Texture;
                Texture* color = renderGraph.GetResource("Color")->Texture;
                Texture* depth = renderGraph.GetResource("Depth")->Texture;

                queue->SetGraphicsPipeline(colorPipeline.Get());
                queue->SetRenderTargets({ color->GetDefaultRTV().Get() }, depth->GetDefaultDSV().Get());

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                queue->ClearRenderTarget(color->GetDefaultRTV().Get(), clearColor);
                queue->ClearDepthStencil(depth->GetDefaultDSV().Get(), 1.0f, 0);

                queue->SetVertexBuffers(0, { bootStrap.GetCubeVertexBuffer() }, { 0 });
                queue->SetIndexBuffer(bootStrap.GetCubeIndexBuffer());

                Light light;
                light.LightDir = lightDir;
                light.LightViewProj = lightProj * lightView;

                void* lightData = lightConstantBuffer->Map();
                memcpy(lightData, &light, sizeof(Light));

                queue->SetTexture("ShadowMap", shadowMap->GetDefaultDSV().Get());
                queue->SetSampler("ShadowSamp", shadowMapSampler.Get());
                queue->SetUniformBuffer("uLight", lightConstantBuffer.Get());

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

                        queue->SetUniformBuffer("uTransform", transformConstantBuffer.Get());

                        queue->DrawIndexed(36, IndexType::Uint32);
                    }
                }

                queue->SetVertexBuffers(0, { bootStrap.GetSquareVertexBuffer() }, { 0 });
                queue->SetIndexBuffer(bootStrap.GetSquareIndexBuffer());

                transform.Model = glm::scale(glm::mat4(1), {40, 1, 40});
                transform.View = camera.GetView();
                transform.Proj = camera.GetProjection();

                void* data = transformConstantBuffer->Map();
                memcpy(data, &transform, sizeof(Transform));

                queue->SetUniformBuffer("uTransform", transformConstantBuffer.Get());

                queue->DrawIndexed(6, IndexType::Uint32);
            });

            renderGraph.AddResource("GodRays", TextureFormat::R16G16B16A16_SFloat, backbufferDesc.Width / 2, backbufferDesc.Height / 2, 
                ResourceBind::RESOURCE_BIND_RENDER_TARGET | ResourceBind::RESOURCE_BIND_SHADER_RESOURSE,
                ResourceState::RenderTarget, ResourceState::ShaderResource);

            renderGraph.AddPass("GodRay", { "ShadowMap", "Depth" }, { "GodRays" }, [&godRaysPipeline, &bootStrap, &sceneConstantBuffer,
                &shadowMapSampler, &sceneDepthSampler, &camera, &lightDir, &lightView, &lightProj, &renderGraph](CommandQueue* queue){
                Texture* shadowMap = renderGraph.GetResource("ShadowMap")->Texture;
                Texture* depth = renderGraph.GetResource("Depth")->Texture;
                Texture* godRays = renderGraph.GetResource("GodRays")->Texture;

                queue->SetGraphicsPipeline(godRaysPipeline.Get());
                queue->SetRenderTargets({ godRays->GetDefaultRTV().Get() }, nullptr);

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                queue->ClearRenderTarget(godRays->GetDefaultRTV().Get(), clearColor);

                queue->SetVertexBuffers(0, { bootStrap.GetFullScreenQuadVertexBuffer() }, { 0 });
                queue->SetIndexBuffer(bootStrap.GetFullScreenQuadIndexBuffer());

                queue->SetSampler("PointClamp", sceneDepthSampler.Get());
                queue->SetSampler("ShadowSamp", shadowMapSampler.Get());
                queue->SetTexture("SceneDepth", depth->GetDefaultSRV().Get());
                queue->SetTexture("ShadowMap", shadowMap->GetDefaultSRV().Get());

                Scene scene;
                scene.CameraPos = camera.GetPosition();
                scene.LightDir = lightDir;
                scene.LightViewProj = lightProj * lightView;
                scene.InvViewProj = glm::inverse(camera.GetProjection() * camera.GetView());

                void* data = sceneConstantBuffer->Map();
                memcpy(data, &scene, sizeof(Scene));

                queue->SetUniformBuffer("uScene", sceneConstantBuffer.Get());

                queue->DrawIndexed(6, IndexType::Uint32);
            });

            renderGraph.AddPass("Composite", { "Color", "GodRays" }, {}, [&compositePipeline, &bootStrap, &backBuffer, 
                &compositeSampler, &renderGraph](CommandQueue* queue){
                Texture* color = renderGraph.GetResource("Color")->Texture;
                Texture* godRays = renderGraph.GetResource("GodRays")->Texture;

                queue->SetGraphicsPipeline(compositePipeline.Get());
                queue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, nullptr);

                queue->SetVertexBuffers(0, { bootStrap.GetFullScreenQuadVertexBuffer() }, { 0 });
                queue->SetIndexBuffer(bootStrap.GetFullScreenQuadIndexBuffer());

                queue->SetTexture("SceneColor", color->GetDefaultSRV().Get());
                queue->SetTexture("GodRays", godRays->GetDefaultSRV().Get());
                queue->SetSampler("LinearClamp", compositeSampler.Get());

                queue->DrawIndexed(6, IndexType::Uint32);
            });

            renderGraph.Compile();
            renderGraph.Execute(commandQueue);

            commandQueue->ResourceBarrier({ 
                {backBuffer, rhi::ResourceState::Present},
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