#pragma once

#include <atomic>
#include <deque>
#include <volk/volk.h>

namespace mad::rhi::vk {

struct DeferredDeleter
{
    void* Data;
    void (*Callback)(void* data, VkDevice device);

    void Destroy(VkDevice device) { Callback(Data, device); }
};

struct ReleaseRefWrapper
{
    DeferredDeleter Deleter;
    std::atomic<int> refs = 1;
};

class ReleaseManager
{
struct StaleEntry
{
    ReleaseRefWrapper* res;
    uint64_t cmdNum;
};

struct ReleaseEntry
{
    ReleaseRefWrapper* res;
    uint64_t fenceValue;
};

public:
    ReleaseManager() = default;
    ~ReleaseManager() = default;

    void Init(VkDevice device);
    void Shutdown();

    void SafeReleaseResource(ReleaseRefWrapper* res, uint64_t cmdNum);
    void DiscardStaleResources(uint64_t cmdNum, uint64_t fenceValue);

    void Purge(uint64_t fenceValue);
    void Flush();

private:
    VkDevice m_Device = nullptr;

    std::deque<StaleEntry> m_StaleQueue;
    std::deque<ReleaseEntry> m_ReleaseQueue;

};

inline DeferredDeleter MakeDeleter(VkBuffer h) 
{
    return { (void*)(uint64_t)h, [](void* d, VkDevice dev) {
        vkDestroyBuffer(dev, (VkBuffer)(uint64_t)d, nullptr);
    }};
};

}