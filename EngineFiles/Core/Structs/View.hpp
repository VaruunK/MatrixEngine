#pragma once

#include "Core/MatrixAPI.hpp"
#include <glm/glm.hpp>

struct MATRIX_API View {
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};