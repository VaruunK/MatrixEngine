#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/ReflectionMacros.hpp"
#include <glm/glm.hpp>
#include "Transform.reflected.hpp"

STRUCT()
struct MATRIX_API Transform {
	REFLECT_STRUCT()

	glm::vec3 location = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
};