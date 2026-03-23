#include <iostream>

#include "Mad-RHI/Factory.h"
#include "Mad-RHI/Device.h"
#include "Mad-RHI/CommandList.h"
#include "Common/Window.h"

using namespace mad::rhi;
using namespace mad::common;

int main()
{
    Window window("Triangle", 800, 600);

    FactoryInitInfo info;
    info.pAppName = "Triangle";
    info.pEngineName = "Mad-RHI";
    info.Backend = FactoryBackend::Vulkan;

    Factory::Init(info);

    {
        Factory* factory = Factory::GetInstance();
        RefPtr<Device> device = nullptr;
        RefPtr<ImmidiateCommandList> icl = nullptr;
        WindowInfo winInfo = window.GetWindowInfo();
        factory->CreateDevice(&device, &icl, winInfo.Connection, winInfo.Window);

        while (window.IsRunning())
        {
            window.Update();
        }
    }

    Factory::Shutdown();

    return 0;
}