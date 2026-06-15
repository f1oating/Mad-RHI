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
    CreateFullScreenQuadBuffers();
    CreateSquareBuffers();
    m_Queue->Flush();
}

void BootStrap::Shutdown()
{
    if (m_SquareVertexBuffer)
    {
        m_SquareVertexBuffer->Release();
    }

    if (m_SquareIndexBuffer)
    {
        m_SquareIndexBuffer->Release();
    }

    if (m_FullScreenQuadVertexBuffer)
    {
        m_FullScreenQuadVertexBuffer->Release();
    }

    if (m_FullScreenQuadIndexBuffer)
    {
        m_FullScreenQuadIndexBuffer->Release();
    }

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
    rhi::CommandQueueDesc queueDesc{};
    queueDesc.Type = rhi::CommandQueueType::COMMAND_QUEUE_TYPE_GRAPHICS;

    rhi::DeviceDesc deviceDesc{};
    deviceDesc.AdapterId = 0;
    deviceDesc.pCommandQueues = &queueDesc;
    deviceDesc.NumCommandQueues = 1;

    m_Factory->CreateDevice(&m_Device, deviceDesc);
    m_Queue = m_Device->GetCommandQueue(0);
}

void BootStrap::CreateSwapchain()
{
    m_Device->CreateSwapchain(&m_Swapchain, m_Window->GetWindowInfo(), m_Queue);
}

void BootStrap::CreateCubeBuffers()
{
    float cubeVertices[] =
    {
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    };

    uint32_t cubeIndices[] =
    {
        0,  1,  2,   0,  2,  3,
        4,  5,  6,   4,  6,  7,
        8,  9, 10,   8, 10, 11,
        12, 13, 14,  12, 14, 15,
        16, 17, 18,  16, 18, 19,
        20, 21, 22,  20, 22, 23,
    };

    rhi::BufferDesc vbd{};
    vbd.Usage = rhi::ResourceUsage::Default;
    vbd.Size = sizeof(cubeVertices);
    vbd.BindFlags = rhi::RESOURCE_BIND_VERTEX_BUFFER;
    m_Device->CreateBuffer(&m_CubeVertexBuffer, vbd);

    rhi::BufferDesc ibd{};
    ibd.Usage = rhi::ResourceUsage::Default;
    ibd.Size = sizeof(cubeIndices);
    ibd.BindFlags = rhi::RESOURCE_BIND_INDEX_BUFFER;
    m_Device->CreateBuffer(&m_CubeIndexBuffer, ibd);

    m_Queue->ResourceBarrier({}, {
        { m_CubeVertexBuffer, rhi::ResourceState::CopyDst },
        { m_CubeIndexBuffer, rhi::ResourceState::CopyDst }
    });

    m_Queue->UpdateBuffer(m_CubeVertexBuffer, cubeVertices, sizeof(cubeVertices));
    m_Queue->UpdateBuffer(m_CubeIndexBuffer, cubeIndices, sizeof(cubeIndices));

    m_Queue->ResourceBarrier({}, {
        { m_CubeVertexBuffer, rhi::ResourceState::VertexBuffer },
        { m_CubeIndexBuffer, rhi::ResourceState::IndexBuffer }
    });
}

void BootStrap::CreateFullScreenQuadBuffers()
{
    float quadVertices[] =
    {
        -1.0f,  1.0f,  0.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 1.0f,
    };

    uint32_t quadIndices[] =
    {
        0, 1, 2,
        0, 2, 3,
    };

    rhi::BufferDesc vbd{};
    vbd.Usage = rhi::ResourceUsage::Default;
    vbd.Size = sizeof(quadVertices);
    vbd.BindFlags = rhi::RESOURCE_BIND_VERTEX_BUFFER;
    m_Device->CreateBuffer(&m_FullScreenQuadVertexBuffer, vbd);

    rhi::BufferDesc ibd{};
    ibd.Usage = rhi::ResourceUsage::Default;
    ibd.Size = sizeof(quadIndices);
    ibd.BindFlags = rhi::RESOURCE_BIND_INDEX_BUFFER;
    m_Device->CreateBuffer(&m_FullScreenQuadIndexBuffer, ibd);

    m_Queue->ResourceBarrier({}, {
        { m_FullScreenQuadVertexBuffer, rhi::ResourceState::CopyDst },
        { m_FullScreenQuadIndexBuffer, rhi::ResourceState::CopyDst }
    });

    m_Queue->UpdateBuffer(m_FullScreenQuadVertexBuffer, quadVertices, sizeof(quadVertices));
    m_Queue->UpdateBuffer(m_FullScreenQuadIndexBuffer, quadIndices, sizeof(quadIndices));

    m_Queue->ResourceBarrier({}, {
        { m_FullScreenQuadVertexBuffer, rhi::ResourceState::VertexBuffer },
        { m_FullScreenQuadIndexBuffer, rhi::ResourceState::IndexBuffer }
    });
}

void BootStrap::CreateSquareBuffers()
{
    float squareVertices[] =
    {
        -0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    };

    uint32_t squareIndices[] =
    {
        0, 1, 2,
        0, 2, 3,
    };

    rhi::BufferDesc vbd{};
    vbd.Usage = rhi::ResourceUsage::Default;
    vbd.Size = sizeof(squareVertices);
    vbd.BindFlags = rhi::RESOURCE_BIND_VERTEX_BUFFER;
    m_Device->CreateBuffer(&m_SquareVertexBuffer, vbd);

    rhi::BufferDesc ibd{};
    ibd.Usage = rhi::ResourceUsage::Default;
    ibd.Size = sizeof(squareIndices);
    ibd.BindFlags = rhi::RESOURCE_BIND_INDEX_BUFFER;
    m_Device->CreateBuffer(&m_SquareIndexBuffer, ibd);

    m_Queue->ResourceBarrier({}, {
        { m_SquareVertexBuffer, rhi::ResourceState::CopyDst },
        { m_SquareIndexBuffer, rhi::ResourceState::CopyDst }
    });

    m_Queue->UpdateBuffer(m_SquareVertexBuffer, squareVertices, sizeof(squareVertices));
    m_Queue->UpdateBuffer(m_SquareIndexBuffer, squareIndices, sizeof(squareIndices));

    m_Queue->ResourceBarrier({}, {
        { m_SquareVertexBuffer, rhi::ResourceState::VertexBuffer },
        { m_SquareIndexBuffer, rhi::ResourceState::IndexBuffer }
    });
}

}