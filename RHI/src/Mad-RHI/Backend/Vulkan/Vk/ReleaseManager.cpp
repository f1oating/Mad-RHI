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

void ReleaseManager::SafeReleaseResource(ReleaseRefWrapper* res, uint64_t cmdNum)
{
    m_StaleQueue.push_back({res, cmdNum});
}

void ReleaseManager::DiscardStaleResources(uint64_t cmdNum, uint64_t fenceValue)
{
    while (!m_StaleQueue.empty() && 
        m_StaleQueue.front().cmdNum <= cmdNum) 
    {
        StaleEntry entry = m_StaleQueue.front();
        m_ReleaseQueue.push_back({entry.res, fenceValue});
        m_StaleQueue.pop_front();
    }
}

void ReleaseManager::Purge(uint64_t fenceValue)
{
    while (!m_ReleaseQueue.empty() && 
        m_ReleaseQueue.front().fenceValue <= fenceValue) 
    {
        ReleaseEntry entry = m_ReleaseQueue.front();
        entry.res->refs--;
        if (entry.res->refs <= 0)
        {
            entry.res->Deleter.Destroy(m_Device);
            delete entry.res;
        }
        m_ReleaseQueue.pop_front();
    }
}

void ReleaseManager::Flush()
{
    for (auto entry : m_StaleQueue)
    {
        entry.res->refs--;
        if (entry.res->refs <= 0)
        {
            entry.res->Deleter.Destroy(m_Device);
            delete entry.res;
        }
    }
    for (auto entry : m_ReleaseQueue)
    {
        entry.res->refs--;
        if (entry.res->refs <= 0)
        {
            entry.res->Deleter.Destroy(m_Device);
            delete entry.res;
        }
    }
    m_StaleQueue.clear();
    m_ReleaseQueue.clear();
}
    
}