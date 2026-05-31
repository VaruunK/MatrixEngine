#pragma once

#include <glm/glm.hpp>

struct View;

class ViewportCamera {
public:
	ViewportCamera();
	~ViewportCamera() = default;

	void MoveUp();
	void MoveDown();
	void MoveLeft();
	void MoveRight();
	void MoveForward();
	void MoveBackward();

	void RotateUp();
	void RotateDown();
	void RotateLeft();
	void RotateRight();

	View GetCameraView();
	
	float GetViewDelta() const { return viewDelta; }
	
	void SetViewDelta(float viewDelta) { this->viewDelta = viewDelta; }
protected:
	float viewDelta = 0.01f;
private:
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	glm::vec3 lookAt;
	glm::vec3 viewRight;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
};