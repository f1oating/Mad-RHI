#include "Common/Event.h"

namespace mad::common {

std::unordered_map<std::type_index, std::vector<std::function<void(const Event&)>>> EventBus::s_Handlers;

void EventBus::Clear()
{
    s_Handlers.clear();
}

}