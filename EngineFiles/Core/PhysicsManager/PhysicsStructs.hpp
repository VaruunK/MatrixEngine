#pragma once

#include "Core/Math/vec2.h"
#include <cstdint>

struct PhysicsState {
	vec2f position;
	vec2f velocity;
	float gravity;
	float mass;
	bool gravityEnabled;
	bool active;
};

struct PhysicsHandle {
	uint32_t index;
	uint32_t generation;
};