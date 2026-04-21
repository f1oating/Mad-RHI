#include <iostream>

#include "Mad-RHI/Factory.h"
#include "Mad-RHI/Device.h"
#include "Mad-RHI/CommandList.h"
#include "Common/Window.h"
#include "Common/Event.h"
#include "Common/ShaderCompiler.h"

using namespace mad;

int main()
{
    common::Window window("Triangle", 800, 600);

    rhi::FactoryInitInfo info;
    info.pAppName = "Triangle";
    info.pEngineName = "Mad-RHI";
    info.Backend = rhi::FactoryBackend::Vulkan;

    rhi::Factory::Init(info);

    {
        rhi::Factory* factory = rhi::Factory::Get();

        rhi::RefPtr<rhi::Device> device = nullptr;
        rhi::RefPtr<rhi::ImmidiateCommandList> icl = nullptr;
        rhi::RefPtr<rhi::Swapchain> swapchain = nullptr;

        common::WindowInfo winInfo = window.GetWindowInfo();

        rhi::WindowHandle wh{};
        wh.platform = rhi::WindowHandle::Platform::XCB;
        wh.xcb.connection = winInfo.Connection;
        wh.xcb.window = winInfo.Window;

        factory->CreateDevice(device.GetAddress(), {});
        icl = device->GetImmidiateCommandList();
        device->CreateSwapchain(swapchain.GetAddress(), wh);

        std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/Vertex.slang" });
        std::vector<uint32_t> spirvFragment = common::ShaderCompiler::Compile({ "shaders/Fragment.slang" });

        rhi::RefPtr<rhi::Shader> vertexShader = nullptr;
        rhi::RefPtr<rhi::Shader> fragmentShader = nullptr; 

        device->CreateShader(vertexShader.GetAddress(), spirvVertex.data(), spirvVertex.size());
        device->CreateShader(fragmentShader.GetAddress(), spirvFragment.data(), spirvFragment.size());

        rhi::GraphicsPipelineDesc pipelineDesc{};
        pipelineDesc.VertexShader   = vertexShader;
        pipelineDesc.FragmentShader = fragmentShader;
        pipelineDesc.Topology = rhi::PrimitiveTopology::TriangleList;
        pipelineDesc.Rasterization.Polygon = rhi::PolygonMode::Fill;
        pipelineDesc.Rasterization.Cull = rhi::CullMode::None;
        pipelineDesc.Rasterization.Face = rhi::FrontFace::CCW;
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
        cbd.Size = 512;
        cbd.BindFlags = rhi::RESOURCE_BIND_UNIFORM_BUFFER;

        rhi::RefPtr<rhi::Buffer> cb = nullptr;
        device->CreateBuffer(cb.GetAddress(), cbd);
        
        rhi::TextureDesc texDesc{};
        texDesc.Dimension = rhi::TextureDimension::Texture2D;
        texDesc.Width  = 512;
        texDesc.Height = 512;
        texDesc.Format = rhi::TextureFormat::RGBA8_UNorm;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.SampleCount = 1;
        texDesc.BindFlags = rhi::RESOURCE_BIND_SHADER_RESOURSE;
        texDesc.Usage = rhi::ResourceUsage::Default;

        rhi::RefPtr<rhi::Texture> texture = nullptr;
        device->CreateTexture(texture.GetAddress(), texDesc);

        rhi::SamplerDesc samplerDesc{};
        samplerDesc.MinFilter = rhi::FilterType::Linear;
        samplerDesc.MagFilter = rhi::FilterType::Linear;
        samplerDesc.MipFilter = rhi::FilterType::Linear;
        samplerDesc.AddressU = rhi::TextureAddressMode::Wrap;
        samplerDesc.AddressV = rhi::TextureAddressMode::Wrap;
        samplerDesc.AddressW = rhi::TextureAddressMode::Wrap;
        samplerDesc.MinLod = 0.0f;
        samplerDesc.MaxLod = 3.402823466e+38f;

        rhi::RefPtr<rhi::Sampler> sampler = nullptr;
        device->CreateSampler(sampler.GetAddress(), samplerDesc);

        rhi::RefPtr<rhi::TextureView> textureSRV = texture->GetDefaultSRV();
        rhi::RefPtr<rhi::TextureView> textureRTV = texture->GetDefaultRTV();
        texture.Reset();
        
        rhi::RefPtr<rhi::Fence> fence = nullptr;
        device->CreateFence(fence.GetAddress());

        struct Vertex { float x, y; float r, g, b; };

        Vertex vertices[] = 
        {
            {  0.0f, -0.5f,  1.0f, 0.0f, 0.0f },
            {  0.5f,  0.5f,  0.0f, 1.0f, 0.0f },
            { -0.5f,  0.5f,  0.0f, 0.0f, 1.0f },
        };

        rhi::BufferDesc vbd{};
        vbd.Usage = rhi::ResourceUsage::Default;
        vbd.Size = sizeof(vertices);
        vbd.BindFlags = rhi::RESOURCE_BIND_VERTEX_BUFFER;
        rhi::RefPtr<rhi::Buffer> vb = nullptr;
        device->CreateBuffer(vb.GetAddress(), vbd);

        icl->ResourceBarrier({}, { {vb.Get(), rhi::ResourceState::CopyDst} });
        icl->UpdateBuffer(vb.Get(), vertices, sizeof(vertices));
        icl->ResourceBarrier({}, { {vb.Get(), rhi::ResourceState::VertexBuffer} });
        icl->Flush();

        while (window.IsRunning())
        {
            window.Update();

            cb->Map();

            rhi::Texture* backBuffer = swapchain->GetCurrentBackBuffer();

            icl->ResourceBarrier({ {backBuffer, rhi::ResourceState::RenderTarget} }, {});

            icl->SetGraphicsPipeline(pipeline.Get());
            icl->SetRenderTargets({ backBuffer->GetDefaultRTV().Get() }, nullptr);

            float clearColor[] = { 0.1f, 0.1f, 0.15f, 1.0f };
            icl->ClearRenderTarget(backBuffer->GetDefaultRTV().Get(), clearColor);
            
            icl->SetVertexBuffers(0, { vb.Get() }, { 0 });
            icl->Draw(3, 0);

            icl->ResourceBarrier({ {backBuffer, rhi::ResourceState::Present} }, {});

            fence->IncrementCurrentValue();
            icl->EnqueueSignal(fence.Get(), fence->GetCurrentValue());

            icl->Flush();

            device->EndFrame();
            device->GarbageCollect();
            swapchain->Present();
        }
    }

    common::EventBus::Clear();
    rhi::Factory::Shutdown();

    return 0;
}