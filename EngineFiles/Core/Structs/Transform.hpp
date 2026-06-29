#pragma once

#include "Core/MatrixAPI.hpp"
#include <glm/glm.hpp>

struct MATRIX_API Transform {
	glm::vec3 location = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
};