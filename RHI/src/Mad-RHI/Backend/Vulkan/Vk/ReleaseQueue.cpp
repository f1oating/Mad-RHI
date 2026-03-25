#include "Mad-RHI/Backend/Vulkan/Vk/ReleaseQueue.h"

namespace mad::rhi::vk {

ReleaseQueue::~ReleaseQueue()
{
    Flush();
}
    
void ReleaseQueue::Enqueue(uint32_t value, std::function<void()> callback)
{
    m_Entries.emplace_back(value, callback);
}

void ReleaseQueue::Poll(uint32_t value)
{
    auto it = m_Entries.begin();
    while (it != m_Entries.end()) 
    {
        if (it->Value <= value) 
        {
            it->Callback();
            it = m_Entries.erase(it);
        } else 
        {
            ++it;
        }
    }
}

void ReleaseQueue::Flush()
{
    for (auto& entry : m_Entries)
        entry.Callback();
}
    
}