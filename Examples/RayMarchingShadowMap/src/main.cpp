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

        RefPtr<GraphicsPipelineState> colorPipeline = nullptr;
        RefPtr<Texture> colorTexture = nullptr;
        RefPtr<Texture> depthTexture = nullptr;
        RefPtr<Buffer> colorConstantBuffer = nullptr;

        RefPtr<GraphicsPipelineState> godraysPipeline = nullptr;
        RefPtr<Texture> godraysTexture = nullptr;
        RefPtr<Buffer> godraysConstantBuffer = nullptr;
        RefPtr<Sampler> godraysSceneDepthSampler = nullptr;
        RefPtr<Sampler> godraysShadowMapSampler = nullptr;

        RefPtr<GraphicsPipelineState> compositePipeline = nullptr;
        RefPtr<Sampler> compositeSampler = nullptr;

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

            BufferDesc colorConstantBufferDesc {};
            colorConstantBufferDesc.BindFlags = ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
            colorConstantBufferDesc.Size = sizeof(Transform);
            colorConstantBufferDesc.Usage = ResourceUsage::Dynamic;
            device->CreateBuffer(colorConstantBuffer.GetAddress(), colorConstantBufferDesc);
        }

        // Godrays pass
        {
            std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/GodraysVertex.slang" });
            std::vector<uint32_t> spirvPixel = common::ShaderCompiler::Compile({ "shaders/GodraysPixel.slang" });
            
            RefPtr<Shader> vertexShader = nullptr;
            RefPtr<Shader> pixelShader = nullptr;

            device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
            device->CreateShader(pixelShader.GetAddress(), spirvPixel.data(), spirvPixel.size());

            GraphicsPipelineDesc godraysPipelineDesc {};
            godraysPipelineDesc.VertexShader = vertexShader;
            godraysPipelineDesc.FragmentShader = pixelShader;
            godraysPipelineDesc.Topology = PrimitiveTopology::TriangleList;
            godraysPipelineDesc.Rasterization.Polygon = PolygonMode::Fill;
            godraysPipelineDesc.Rasterization.Cull = CullMode::None;
            godraysPipelineDesc.Rasterization.Face = FrontFace::CCW;
            godraysPipelineDesc.DepthStencil.DepthTestEnable = false;
            godraysPipelineDesc.DepthStencil.DepthWriteEnable = false;
            ColorAttachmentBlend colorBlend{};
            colorBlend.BlendEnable = false;
            godraysPipelineDesc.BlendAttachments.push_back(colorBlend);
            godraysPipelineDesc.Rendering.ColorFormats.push_back(TextureFormat::RGBA16_Float);
            godraysPipelineDesc.Rendering.SampleCount = 1;
            device->CreateGraphicsPipeline(godraysPipeline.GetAddress(), godraysPipelineDesc);

            TextureDesc godraysTextureDesc {};
            godraysTextureDesc.Width = 400;
            godraysTextureDesc.Height = 300;
            godraysTextureDesc.BindFlags = RESOURCE_BIND_RENDER_TARGET | RESOURCE_BIND_SHADER_RESOURSE;
            godraysTextureDesc.Format = TextureFormat::RGBA16_Float;
            device->CreateTexture(godraysTexture.GetAddress(), godraysTextureDesc);

            SamplerDesc godraysSceneDepthSamplerDesc {};
            godraysSceneDepthSamplerDesc.MinFilter = FilterType::Nearest;
            godraysSceneDepthSamplerDesc.MagFilter = FilterType::Nearest;
            godraysSceneDepthSamplerDesc.MipFilter = FilterType::Nearest;
            godraysSceneDepthSamplerDesc.AddressU = TextureAddressMode::ClampToEdge;
            godraysSceneDepthSamplerDesc.AddressV = TextureAddressMode::ClampToEdge;
            device->CreateSampler(godraysSceneDepthSampler.GetAddress(), godraysSceneDepthSamplerDesc);

            SamplerDesc godraysShadowMapSamplerDesc {};
            godraysShadowMapSamplerDesc.MinFilter = FilterType::Linear;
            godraysShadowMapSamplerDesc.MagFilter = FilterType::Linear;
            godraysShadowMapSamplerDesc.MipFilter = FilterType::Nearest;
            godraysShadowMapSamplerDesc.AddressU = TextureAddressMode::ClampToBorder;
            godraysShadowMapSamplerDesc.AddressV = TextureAddressMode::ClampToBorder;
            godraysShadowMapSamplerDesc.Border = BorderColor::FloatOpaqueWhite;
            godraysShadowMapSamplerDesc.Compare = CompareOp::LessEqual;
            device->CreateSampler(godraysShadowMapSampler.GetAddress(), godraysShadowMapSamplerDesc);
            
            BufferDesc godraysConstantBufferDesc {};
            godraysConstantBufferDesc.BindFlags = ResourceBind::RESOURCE_BIND_UNIFORM_BUFFER;
            godraysConstantBufferDesc.Size = sizeof(Scene);
            godraysConstantBufferDesc.Usage = ResourceUsage::Dynamic;
            device->CreateBuffer(godraysConstantBuffer.GetAddress(), godraysConstantBufferDesc);
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

        glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
        glm::vec3 lightPos = -lightDir * 50.0f;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), up);

        float size = 50.0f;
        float near = 0.1f;
        float far = 200.0f;

        glm::mat4 lightProj = glm::ortho(-size, size, -size, size, near, far);

        common::Camera camera { { 0.0f, 1.0f, 3.0f }, 90.0f, 800.0f / 600.0f, 0.1f, 100.0f };

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
                {godraysTexture.Get(), ResourceState::RenderTarget},
            }, {});

            // Shadow map pass
            {
                commandQueue->SetGraphicsPipeline(shadowMapPipeline.Get());
                commandQueue->SetRenderTargets({ }, shadowMapTexture->GetDefaultDSV().Get());

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

                Transform transform;
                transform.Model = glm::mat4(1.0f);
                transform.View = camera.GetView();
                transform.Proj = camera.GetProjection();

                void* data = colorConstantBuffer->Map();
                memcpy(data, &transform, sizeof(Transform));

                commandQueue->SetUniformBuffer("uTransform", colorConstantBuffer.Get());

                commandQueue->DrawIndexed(36, IndexType::Uint32);

                commandQueue->ResourceBarrier({ {colorTexture.Get(), ResourceState::ShaderResource}, {depthTexture.Get(), ResourceState::ShaderResource} }, {});
            }

            // Godrays pass
            {
                commandQueue->SetGraphicsPipeline(godraysPipeline.Get());
                commandQueue->SetRenderTargets({ godraysTexture->GetDefaultRTV().Get() }, nullptr);

                float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                commandQueue->ClearRenderTarget(godraysTexture->GetDefaultRTV().Get(), clearColor);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetFullScreenQuadVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetFullScreenQuadIndexBuffer());

                commandQueue->SetSampler("PointClamp", godraysSceneDepthSampler.Get());
                commandQueue->SetSampler("ShadowSamp", godraysShadowMapSampler.Get());
                commandQueue->SetTexture("SceneDepth", depthTexture->GetDefaultSRV().Get());
                commandQueue->SetTexture("ShadowMap", shadowMapTexture->GetDefaultSRV().Get());

                Scene scene;
                scene.CameraPos = camera.GetPosition();
                scene.LightDir = lightDir;
                scene.LightViewProj = lightProj * lightView;
                scene.InvViewProj = glm::inverse(camera.GetProjection() * camera.GetView());

                void* data = godraysConstantBuffer->Map();
                memcpy(data, &scene, sizeof(Scene));

                commandQueue->SetUniformBuffer("uScene", godraysConstantBuffer.Get());

                commandQueue->DrawIndexed(6, IndexType::Uint32);

                commandQueue->ResourceBarrier({ {godraysTexture.Get(), ResourceState::ShaderResource} }, {});
            }

            // Composite pass
            {
                commandQueue->SetGraphicsPipeline(compositePipeline.Get());
                commandQueue->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, nullptr);

                commandQueue->SetVertexBuffers(0, { bootStrap.GetFullScreenQuadVertexBuffer() }, { 0 });
                commandQueue->SetIndexBuffer(bootStrap.GetFullScreenQuadIndexBuffer());

                commandQueue->SetTexture("SceneColor", colorTexture->GetDefaultSRV().Get());
                commandQueue->SetTexture("Godrays", godraysTexture->GetDefaultSRV().Get());
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