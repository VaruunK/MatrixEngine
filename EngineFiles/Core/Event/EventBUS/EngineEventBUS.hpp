#pragma once

// this class will NOT be included in the game eventually, once a legitimate game event system is made
// remove the MATRIX_API 

#include "Core/MatrixAPI.hpp"
#include <functional>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <mutex>
#include <SDL3/SDL_events.h>

enum {
	EVENT_VIEWPORT_CLICKED = 0x8001,
	EVENT_VIEWPORT_HOVERED = 0x8002,
	EVENT_CONTENT_BROWSER_HOVERED = 0x8003,

	EVENT_GAME_START = 0x8004,
	EVENT_GAME_END = 0x8005,
};

class MATRIX_API EngineEventBUS {
public:
	EngineEventBUS();
	void Subscribe(uint32_t eventType, std::function<void()> callback);
	void Notify(uint32_t eventType);

	void ProcessEvent(SDL_Event* event);
private:
	
	std::map<uint32_t, std::list<std::function<void()>>> eventMappings;
};

inline EngineEventBUS GEventBUS;