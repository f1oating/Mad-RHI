#include "Common/BootStrap.h"

namespace mad::common {

void BootStrap::Init(const char* appName)
{
    m_Window = new Window(appName, 800, 600);

    rhi::FactoryInitInfo info;
    info.pAppName = appName;
    info.pEngineName = "Mad-RHI";
    info.Backend = rhi::FactoryBackend::Vulkan;

    rhi::Factory::Init(info);
    m_Factory = rhi::Factory::Get();

    CreateDeviceAndQueue();
    CreateSwapchain();
    CreateCubeBuffers();
}

void BootStrap::Shutdown()
{
    if (m_CubeVertexBuffer)
    {
        m_CubeVertexBuffer->Release();
    }

    if (m_CubeIndexBuffer)
    {
        m_CubeIndexBuffer->Release();
    }

    if (m_Swapchain)
    {
        m_Swapchain->Release();
    }

    if (m_Device)
    {
        m_Device->Release();
    }

    Factory::Shutdown();
    m_Factory = nullptr;
}

void BootStrap::CreateDeviceAndQueue()
{
    uint32_t numAdapters = 0;
    m_Factory->EnumerateAdapters(numAdapters, nullptr);

    std::vector<rhi::AdapterInfo> adapters(numAdapters);
    m_Factory->EnumerateAdapters(numAdapters, adapters.data());

    const rhi::AdapterInfo& adapter = adapters[0];
    uint32_t graphicsFamilyIndex = UINT32_MAX;

    for (uint32_t i = 0; i < adapter.NumFamilies; i++)
    {
        if (adapter.Families[i].Flags & rhi::COMMAND_QUEUE_TYPE_GRAPHICS_BIT)
        {
            graphicsFamilyIndex = adapter.Families[i].Index;
            break;
        }
    }

    rhi::CommandQueueDesc queueDesc{};
    queueDesc.Index = graphicsFamilyIndex;
    queueDesc.Flags = rhi::COMMAND_QUEUE_TYPE_GRAPHICS_BIT;

    rhi::DeviceDesc deviceDesc{};
    deviceDesc.AdapterId = 0;
    deviceDesc.pCommandQueues = &queueDesc;
    deviceDesc.NumCommandQueues = 1;

    m_Factory->CreateDevice(&m_Device, deviceDesc);
    m_Queue = m_Device->GetCommandQueue(0);
}

void BootStrap::CreateSwapchain()
{
    WindowInfo winInfo = m_Window->GetWindowInfo();

    WindowHandle wh{};
    wh.platform = WindowHandle::Platform::XCB;
    wh.xcb.connection = winInfo.Connection;
    wh.xcb.window = winInfo.Window;

    m_Device->CreateSwapchain(&m_Swapchain, wh, m_Queue);
}

void BootStrap::CreateCubeBuffers()
{

}

}