#include <iostream>

#include "Mad-RHI/Factory.h"
#include "Mad-RHI/Device.h"

using namespace mad::rhi;

int main()
{
    FactoryInitInfo info;
    info.pAppName = "Triangle";
    info.pEngineName = "Mad-RHI";
    info.Backend = FactoryBackend::Vulkan;

    Factory::Init(info);

    Factory* factory = Factory::GetInstance();
    RefPtr<Device> device = nullptr;
    factory->CreateDevice(&device);

    Factory::Shutdown();

    return 0;
}