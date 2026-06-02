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
	UpdateView();
}

void ViewportCamera::MoveForward() {
	eye += lookAt * viewDelta;
	center = eye + lookAt;
	UpdateView();
}

void ViewportCamera::MoveBackward() {
	eye -= lookAt * viewDelta;
	center = eye + lookAt;
	UpdateView();
}

void ViewportCamera::MoveLeft() {
	eye -= viewRight * viewDelta;
	center = eye + lookAt;
	UpdateView();
}

void ViewportCamera::MoveRight() {
	eye += viewRight * viewDelta;
	center = eye + lookAt;
	UpdateView();
}

void ViewportCamera::MoveUp() {
	eye += up * viewDelta;
	center = eye + lookAt;
	UpdateView();
}

void ViewportCamera::MoveDown() {
	eye -= up * viewDelta;
	center = eye + lookAt;
	UpdateView();
}

void ViewportCamera::RotateByMouse(float relX, float relY) {
	yaw += relX * mouseSensitivity;
	pitch -= relY * mouseSensitivity;

	pitch = glm::clamp(pitch, -89.0f, 89.0f);

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	lookAt = glm::normalize(direction);
	center = eye + lookAt;
	viewRight = glm::normalize(glm::cross(lookAt, glm::vec3(0, 1, 0)));
	up = glm::normalize(glm::cross(viewRight, lookAt));

	UpdateView();
}

void ViewportCamera::UpdateView() {
	view.projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f);
	view.viewMatrix = glm::lookAt(eye, center, up);
}

const View& ViewportCamera::GetCameraView() const {
	return view;
}