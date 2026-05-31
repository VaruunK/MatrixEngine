#include "ViewportCamera.hpp"
#include "Core/Structs/View.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

ViewportCamera::ViewportCamera() {
	eye = glm::vec3(0.0f, 0.0f, 3.0f);
	center = glm::vec3(0.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	lookAt = glm::normalize(center - eye);
	viewRight = glm::normalize(glm::cross(lookAt, up));
	projectionMatrix = glm::perspective(glm::radians(0.5f), 1.0f, 0.1f, 1000.0f);
	viewMatrix = glm::lookAt(eye, center, up);
}

void ViewportCamera::MoveUp() {
	eye += up * viewDelta;
	center += up * viewDelta;
}

void ViewportCamera::MoveDown() {
	eye += up * -viewDelta;
	center += up * -viewDelta;
}

void ViewportCamera::MoveLeft() {
	eye += lookAt * viewDelta;
	center += viewRight * viewDelta;
}

void ViewportCamera::MoveRight() {
	eye += lookAt * -viewDelta;
	center += viewRight * -viewDelta;
}

void ViewportCamera::MoveForward() {
	eye += lookAt * viewDelta;
	center += lookAt * viewDelta;
}

void ViewportCamera::MoveBackward() {
	eye += lookAt * -viewDelta;
	center += lookAt * -viewDelta;
}

void ViewportCamera::RotateUp() {
	
}

void ViewportCamera::RotateDown() {
}

void ViewportCamera::RotateLeft() {
	up = glm::normalize(up + (viewRight * -viewDelta));
}

void ViewportCamera::RotateRight() {
	up = glm::normalize(up + (viewRight * viewDelta));
}

View ViewportCamera::GetCameraView() {
	View view = {};
	view.projectionMatrix = glm::perspective(glm::radians(0.5f), 1.0f, 0.1f, 1000.0f);
	view.viewMatrix = glm::lookAt(eye, center, up);
	return view;
}