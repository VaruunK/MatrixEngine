#include "EngineEventBUS.hpp"

EngineEventBUS::EngineEventBUS() {
    SDL_RegisterEvents(4);
}

void EngineEventBUS::Subscribe(uint32_t eventType, std::function<void()> callback) {
    eventMappings[eventType].push_back(callback);
}

void EngineEventBUS::Notify(uint32_t eventType) {
    SDL_Event event{
        .type = (eventType)
    };

    SDL_PushEvent(&event);
}

void EngineEventBUS::ProcessEvent(SDL_Event *event) {
    auto it = eventMappings.find(event->type);
    if (it != eventMappings.end()) {
        for (auto& callback : it->second) {
            callback();
        }
    }
}