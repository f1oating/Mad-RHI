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

inline DeferredDeleter MakeDeleter(VkBuffer h) 
{
    return { (void*)(uint64_t)h, [](void* d, VkDevice dev) {
        vkDestroyBuffer(dev, (VkBuffer)(uint64_t)d, nullptr);
    }};
};

class ReleaseManager
{
struct PendingDelete
{
    DeferredDeleter Deleter;
    uint64_t FenceValue;
};

struct RefWrapper
{
    PendingDelete resource;
    std::atomic<int> refs = 1;
};

public:
    ReleaseManager() = default;
    ~ReleaseManager() = default;

    void Init(VkDevice device);
    void Shutdown();

    void Enqueue(DeferredDeleter deleter, uint64_t fenceValue);

    void Purge(uint64_t fenceValue);
    void Flush();

private:
    VkDevice m_Device = nullptr;

    std::deque<RefWrapper*> m_GraphicsReleaseQueue;

};

}