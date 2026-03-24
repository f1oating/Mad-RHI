#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>

namespace mad::common {

struct Event
{
public:
    virtual ~Event() = default;

};

class EventBus
{
private:
    static std::unordered_map<std::type_index, std::vector<std::function<void(const Event&)>>> s_Handlers;

public:
    template<typename T>
    static void Subscribe(std::function<void(const T&)> handler)
    {
        s_Handlers[typeid(T)].push_back(
            [handler](const Event& e) { handler(static_cast<const T&>(e)); }
        );
    }

    template<typename T>
    static void Emit(const T& event)
    {
        auto it = s_Handlers.find(typeid(T));
        if (it == s_Handlers.end()) return;
        for (auto handler : it->second)
        {
            handler(event);
        }
    }

    static void Clear();

};

struct WindowResizeEvent : Event
{
    int Width; 
    int Height;
};

}