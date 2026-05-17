#include "Mad-RHI/Factory.h"

#include "Mad-RHI/Backend/Vulkan/VulkanFactory.h"

namespace mad::rhi {

Factory* Factory::s_Factory = nullptr;
bool Factory::s_IsInitialized = false;

void Factory::Init(const FactoryInitInfo& info)
{
    if (s_IsInitialized) return;

    switch (info.Backend)
    {
    case FactoryBackend::Vulkan:
        s_Factory = new VulkanFactory(info);
        break;
    
    default:
        s_Factory = new VulkanFactory(info);
        break;
    }

    s_IsInitialized = true;
}

void Factory::Shutdown()
{
    if (!s_IsInitialized) return;
    if (s_Factory) delete s_Factory;
}

Factory* Factory::Get()
{
    return s_Factory;
}

}