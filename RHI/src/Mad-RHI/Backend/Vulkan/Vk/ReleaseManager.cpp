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

void ReleaseManager::SafeReleaseResource(StaleResourceWrapper&& wrapper, uint64_t cmdNum)
{
    m_StaleQueue.emplace_back(cmdNum, std::move(wrapper));
}

void ReleaseManager::SafeReleaseResource(const StaleResourceWrapper& wrapper, uint64_t cmdNum)
{
    m_StaleQueue.emplace_back(cmdNum, wrapper);
}

void ReleaseManager::DiscardStaleResources(uint64_t fenceValue)
{
    while (!m_StaleQueue.empty())
    {
        m_ReleaseQueue.emplace_back(fenceValue, std::move(m_StaleQueue.front().second));
        m_StaleQueue.pop_front();
    }
}

void ReleaseManager::DiscardStaleResources(uint64_t cmdNum, uint64_t fenceValue)
{
    while (!m_StaleQueue.empty() &&
        m_StaleQueue.front().first <= cmdNum)
    {
        m_ReleaseQueue.emplace_back(fenceValue, std::move(m_StaleQueue.front().second));
        m_StaleQueue.pop_front();
    }
}

void ReleaseManager::Purge(uint64_t fenceValue)
{
    while (!m_ReleaseQueue.empty() &&
        m_ReleaseQueue.front().first <= fenceValue)
    {
        m_ReleaseQueue.pop_front();
    }
}

void ReleaseManager::Flush()
{
    m_StaleQueue.clear();
    m_ReleaseQueue.clear();
}

}