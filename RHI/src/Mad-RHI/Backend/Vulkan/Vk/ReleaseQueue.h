#pragma once

#include <functional>
#include <cstdint>
#include <deque>

namespace mad::rhi::vk {

class ReleaseQueue
{

struct Entry
{
    uint32_t Value;
    std::function<void()> Callback;
};

public:
    ReleaseQueue() = default;
    ~ReleaseQueue();

    void Enqueue(uint32_t value, std::function<void()> callback);

    void Poll(uint32_t value);
    void Flush();

private:
    std::deque<Entry> m_Entries;

};

}