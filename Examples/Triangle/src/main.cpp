#include <iostream>

#include "Mad-RHI/Factory.h"
#include "Mad-RHI/Device.h"
#include "Mad-RHI/CommandList.h"
#include "Common/Window.h"
#include "Common/Event.h"

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
        factory->CreateDevice(&device, &icl, wh);

        common::EventBus::Subscribe<common::WindowResizeEvent>([&device](const common::WindowResizeEvent& event) {
            device->Resize();
        });

        while (window.IsRunning())
        {
            window.Update();
        }
    }

    common::EventBus::Clear();
    rhi::Factory::Shutdown();

    return 0;
}