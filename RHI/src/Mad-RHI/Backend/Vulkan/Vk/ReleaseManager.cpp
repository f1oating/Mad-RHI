#include "Mad-RHI/Backend/Vulkan/Vk/ReleaseManager.h"

namespace mad::rhi::vk {

void ReleaseManager::Init(VkDevice device)
{
    m_Device = device;
}

void ReleaseManager::Shutdown()
{
    Flush();
}

void ReleaseManager::Enqueue(DeferredDeleter deleter, uint64_t fenceValue)
{
    RefWrapper* ref = new RefWrapper();
    ref->resource = { deleter, fenceValue };
    m_GraphicsReleaseQueue.push_back(ref);
}

void ReleaseManager::Purge(uint64_t fenceValue)
{
    while (!m_GraphicsReleaseQueue.empty() && 
        m_GraphicsReleaseQueue.front()->resource.FenceValue <= fenceValue) 
    {
        RefWrapper* ref = m_GraphicsReleaseQueue.front();
        ref->resource.Deleter.Destroy(m_Device);
        delete ref;
        m_GraphicsReleaseQueue.pop_front();
    }
}

void ReleaseManager::Flush()
{
    for (auto ref : m_GraphicsReleaseQueue)
    {
        ref->resource.Deleter.Destroy(m_Device);
        delete ref;
    }
    m_GraphicsReleaseQueue.clear();
}
    
}