#pragma once

#include "Mad-RHI/Factory.h"
#include "Common/Window.h"

namespace mad::common {

using namespace rhi;

class BootStrap
{
public:
    void Init(const char* appName);
    void Shutdown();

    Window* GetWindow() { return m_Window; }

    Factory* GetFactory() { return m_Factory; }
    Device* GetDevice() { return m_Device; }
    CommandQueue* GetQueue() { return m_Queue; }
    Swapchain* GetSwapchain() { return m_Swapchain; }

    Buffer* GetCubeVertexBuffer() { return m_CubeVertexBuffer; }
    Buffer* GetCubeIndexBuffer() { return m_CubeIndexBuffer; }

    Buffer* GetFullScreenQuadVertexBuffer() { return m_FullScreenQuadVertexBuffer; }
    Buffer* GetFullScreenQuadIndexBuffer() { return m_FullScreenQuadIndexBuffer; }

private:
    Window* m_Window = nullptr;

    Factory* m_Factory = nullptr;
    Device* m_Device = nullptr;
    CommandQueue* m_Queue = nullptr;
    Swapchain* m_Swapchain = nullptr;

    Buffer* m_CubeVertexBuffer = nullptr;
    Buffer* m_CubeIndexBuffer = nullptr;

    Buffer* m_FullScreenQuadVertexBuffer = nullptr;
    Buffer* m_FullScreenQuadIndexBuffer = nullptr;

private:
    void CreateDeviceAndQueue();
    void CreateSwapchain();
    void CreateCubeBuffers();
    void CreateFullScreenQuadBuffers();

};

}