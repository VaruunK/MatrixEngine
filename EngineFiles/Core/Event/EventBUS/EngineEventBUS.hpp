#pragma once
#include <functional>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <mutex>
#include <SDL3/SDL_events.h>

class EngineEventBUS {
public:
	void Subscribe(SDL_EventType eventType, std::function<void()> callback);

	void ProcessEvent(SDL_Event* event);
private:
	
	std::map<SDL_EventType, std::list<std::function<void()>>> eventMappings;
};

inline EngineEventBUS GEventBUS;