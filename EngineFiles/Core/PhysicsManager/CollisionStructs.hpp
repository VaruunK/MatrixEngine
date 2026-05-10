#pragma once

#include "Core/Math/vec2.h"
#include <unordered_map>
#include <cstdint>
#include <typeindex>
#include <type_traits>
#include <SDL3/SDL.h>

enum class CollisionType {
	OVERLAP,
	HIT,
	IGNORE
};

struct CollisionState {
	SDL_FRect collisionBox;
	vec2f bounds;
	std::unordered_map<std::type_index, CollisionType> collisionResponses;
	bool active;
};

struct CollisionHandle {
	uint32_t index;
	uint32_t generation;
};

struct CollisionResponse {
	std::type_index type;
	CollisionType response;
};