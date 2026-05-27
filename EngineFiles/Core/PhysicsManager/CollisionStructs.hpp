#pragma once
#include <unordered_map>
#include <cstdint>
#include <typeindex>
#include <type_traits>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>

enum class CollisionType {
	OVERLAP,
	HIT,
	IGNORE
};

struct CollisionState {
	SDL_FRect collisionBox;
	glm::vec2 bounds;
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