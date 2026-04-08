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
        rhi::Factory* factory = rhi::Factory::GetInstance();

        rhi::RefPtr<rhi::Device> device = nullptr;
        rhi::RefPtr<rhi::ImmidiateCommandList> icl = nullptr;

        common::WindowInfo winInfo = window.GetWindowInfo();

        rhi::WindowHandle wh{};
        wh.platform = rhi::WindowHandle::Platform::XCB;
        wh.xcb.connection = winInfo.Connection;
        wh.xcb.window = winInfo.Window;
        factory->CreateDevice(&device, wh);
        icl = device->GetImmidiateCommandList();

        std::vector<uint32_t> spirvVertex = common::ShaderCompiler::Compile({ "shaders/Vertex.slang" });
        std::vector<uint32_t> spirvFragment = common::ShaderCompiler::Compile({ "shaders/Fragment.slang" });

        rhi::RefPtr<rhi::Shader> vertexShader = device->CreateShader(spirvVertex.data(), spirvVertex.size());
        rhi::RefPtr<rhi::Shader> fragmentShader = device->CreateShader(spirvFragment.data(), spirvFragment.size());

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
        rhi::RefPtr<rhi::GraphicsPipelineState> pipeline = device->CreateGraphicsPipeline(pipelineDesc);

        rhi::BufferDesc ibd{};
        ibd.Usage = rhi::ResourceUsage::Default;
        ibd.Size = 12;
        ibd.BindFlags = rhi::ResourceBindFlags::IndexBuffer;

        rhi::RefPtr<rhi::Buffer> ib = device->CreateBuffer(ibd);

        rhi::TextureDesc texDesc{};
        texDesc.Name = "MyTexture";
        texDesc.Dimension = rhi::TextureDimension::Texture2D;
        texDesc.Width  = 512;
        texDesc.Height = 512;
        texDesc.Format = rhi::TextureFormat::RGBA8_UNorm;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.SampleCount = 1;
        texDesc.BindFlags = rhi::ShaderResource;
        texDesc.Usage = rhi::ResourceUsage::Default;

        rhi::RefPtr<rhi::Texture> texture = device->CreateTexture(texDesc);

        common::EventBus::Subscribe<common::WindowResizeEvent>([&device](const common::WindowResizeEvent& event) {
            device->Resize();
        });

        while (window.IsRunning())
        {
            window.Update();
            device->EndFrame();
            device->Present();
        }
    }

    common::EventBus::Clear();
    rhi::Factory::Shutdown();

    return 0;
}