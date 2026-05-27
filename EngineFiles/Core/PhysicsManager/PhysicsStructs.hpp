#pragma once


#include <cstdint>
#include <glm/glm.hpp>

struct PhysicsState {
	glm::vec2 position;
	glm::vec2 velocity;
	float gravity;
	float mass;
	bool gravityEnabled;
	bool active;
};

struct PhysicsHandle {
	uint32_t index;
	uint32_t generation;
};