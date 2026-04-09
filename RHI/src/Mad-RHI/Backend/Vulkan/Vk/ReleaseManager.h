#pragma once

#include <atomic>
#include <deque>
#include <volk/volk.h>
#include <vk_mem_alloc.h>

namespace mad::rhi::vk {

struct StaleResourceBase
{
    std::atomic<int> RefCount { 1 };
    virtual ~StaleResourceBase() = default;
    virtual void Destroy() = 0;

    void Release()
    {
        if (--RefCount == 0)
        {
            Destroy();
            delete this;
        }
    }
};

class StaleResourceWrapper
{
public:
    static StaleResourceWrapper Create(StaleResourceBase* p, int numRefs = 1)
    {
        p->RefCount.store(numRefs);
        return StaleResourceWrapper(p);
    }

    StaleResourceWrapper(const StaleResourceWrapper& o) : m_Ptr(o.m_Ptr) {}
    StaleResourceWrapper(StaleResourceWrapper&& o) : m_Ptr(o.m_Ptr) { o.m_Ptr = nullptr; }

    StaleResourceWrapper& operator=(const StaleResourceWrapper&) = delete;
    StaleResourceWrapper& operator=(StaleResourceWrapper&&) = delete;

    ~StaleResourceWrapper() { if (m_Ptr) m_Ptr->Release(); }

    void GiveUpOwnership() { m_Ptr = nullptr; }

private:
    StaleResourceWrapper(StaleResourceBase* p) : m_Ptr(p) {}
    StaleResourceBase* m_Ptr = nullptr;
};

class ReleaseManager
{
using Entry = std::pair<uint64_t, StaleResourceWrapper>;

public:
    void Init(VkDevice device);
    void Shutdown();

    void SafeReleaseResource(StaleResourceWrapper&& wrapper, uint64_t cmdNum);
    void SafeReleaseResource(const StaleResourceWrapper& wrapper, uint64_t cmdNum);

    void DiscardStaleResources(uint64_t fenceValue);
    void DiscardStaleResources(uint64_t cmdNum, uint64_t fenceValue);

    void Purge(uint64_t fenceValue);

    void Flush();

private:
    VkDevice m_Device = nullptr;

    std::deque<Entry> m_StaleQueue;
    std::deque<Entry> m_ReleaseQueue;

};

struct VkBufferResource : StaleResourceBase
{
    VkBuffer Buffer;
    VmaAllocation Allocation;
    VmaAllocator Allocator;

    VkBufferResource(VkBuffer b, VmaAllocation a, VmaAllocator al)
        : Buffer(b), Allocation(a), Allocator(al) {}

    void Destroy() override
    {
        vmaDestroyBuffer(Allocator, Buffer, Allocation);
    }
};

struct VkImageResource : StaleResourceBase 
{
    VkImage Image;
    VmaAllocation Allocation;
    VmaAllocator Allocator;
    
    VkImageResource(VkImage i, VmaAllocation a, VmaAllocator al)
        : Image(i), Allocation(a), Allocator(al) {}

    void Destroy() override { vmaDestroyImage(Allocator, Image, Allocation); }
};


}