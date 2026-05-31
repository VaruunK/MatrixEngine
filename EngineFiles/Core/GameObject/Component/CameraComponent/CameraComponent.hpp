#pragma once

#include "Core/GameObject/Component/Component.hpp"
#include <glm/glm.hpp>

class CameraComponent : public Component {
public:
protected:
	float viewDelta = 0.01f;
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	glm::vec3 lookAt;
	glm::vec3 viewRight;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
};