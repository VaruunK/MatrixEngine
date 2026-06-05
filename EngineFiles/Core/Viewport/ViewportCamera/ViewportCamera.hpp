#pragma once
#include "Core/Structs/View.hpp"
#include <glm/glm.hpp>

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

	void RotateByMouse(float relX, float relY);

	const View& GetCameraView() const;
	
	float GetViewDelta() const { return viewDelta; }
	
	void SetCameraSpeed(int& speed);
private:
	void UpdateView();

	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	glm::vec3 lookAt;
	glm::vec3 viewRight;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	int cameraSpeed = 1;
	float viewDelta = 0.01f;

	float yaw = -90.0f;
	float pitch = 0.0f;
	float mouseSensitivity = 0.1f;

	View view;
};