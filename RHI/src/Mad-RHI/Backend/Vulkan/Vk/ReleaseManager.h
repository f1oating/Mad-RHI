#pragma once

#include <atomic>
#include <deque>
#include <volk/volk.h>

namespace mad::rhi::vk {

struct StaleResourceBase
{
    virtual ~StaleResourceBase() = default;
    virtual void Release() = 0;
};

template<typename T>
struct StaleResourceOwned final : StaleResourceBase
{
    T Resource;
    StaleResourceOwned(T&& r) : Resource(std::move(r)) {};
    void Release() override { delete this; }
};

template<typename T>
struct StaleResourceShared : StaleResourceBase
{
    T Resource;
    std::atomic<int> RefCount;
    StaleResourceShared(T&& r, int n) : Resource(std::move(r)), RefCount(n) {}
    void Release() override { if (--RefCount == 0) delete this; }
};

class StaleResourceWrapper
{
public:
    template<typename T>
    static StaleResourceWrapper Create(T&& resource, int numRefs = 1)
    {
        StaleResourceBase* p = (numRefs == 1)
            ? static_cast<StaleResourceBase*>(new StaleResourceOwned<T>(std::move(resource)))
            : static_cast<StaleResourceBase*>(new StaleResourceShared<T>(std::move(resource), numRefs));
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

    template<typename T>
    void SafeReleaseResource(T&& resource, uint64_t cmdNum)
    {
        SafeReleaseResource(StaleResourceWrapper::Create(std::move(resource)), cmdNum);
    }

    void SafeReleaseResource(StaleResourceWrapper&& wrapper, uint64_t cmdNum);
    void SafeReleaseResource(const StaleResourceWrapper& wrapper, uint64_t cmdNum);

    void DiscardStaleResources(uint64_t cmdNum, uint64_t fenceValue);

    void Purge(uint64_t fenceValue);

    void Flush();

private:
    VkDevice m_Device = nullptr;

    std::deque<Entry> m_StaleQueue;
    std::deque<Entry> m_ReleaseQueue;

};

}