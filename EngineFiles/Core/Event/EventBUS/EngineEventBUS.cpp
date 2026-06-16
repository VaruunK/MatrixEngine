#include "EngineEventBUS.hpp"

void EngineEventBUS::Subscribe(SDL_EventType eventType, std::function<void()> callback) {
    eventMappings[eventType].push_back(callback);
}

void EngineEventBUS::ProcessEvent(SDL_Event* event) {
    auto it = eventMappings.find(static_cast<SDL_EventType>(event->type));
    if (it != eventMappings.end()) {
        for (auto& callback : it->second) {
            callback();
        }
    }
}