#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/ReflectionMacros.hpp"
#include <glm/glm.hpp>
#include "View.reflected.hpp"

STRUCT()
struct MATRIX_API View {

	REFLECT_STRUCT()
	
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};