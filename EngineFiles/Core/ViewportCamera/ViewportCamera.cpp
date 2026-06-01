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

void ViewportCamera::MoveUp() {
	eye += up * viewDelta;
	center += up * viewDelta;
	UpdateView();
}

void ViewportCamera::MoveDown() {
	eye += up * -viewDelta;
	center += up * -viewDelta;
	UpdateView();
}

void ViewportCamera::MoveLeft() {
	eye += lookAt * viewDelta;
	center += viewRight * viewDelta;
	UpdateView();
}

void ViewportCamera::MoveRight() {
	eye += lookAt * -viewDelta;
	center += viewRight * -viewDelta;
	UpdateView();
}

void ViewportCamera::MoveForward() {
	eye += lookAt * viewDelta;
	center += lookAt * viewDelta;
	UpdateView();
}

void ViewportCamera::MoveBackward() {
	eye += lookAt * -viewDelta;
	center += lookAt * -viewDelta;
	UpdateView();
}

void ViewportCamera::RotateUp() {
	UpdateView();
}

void ViewportCamera::RotateDown() {
	UpdateView();
}

void ViewportCamera::RotateLeft() {
	up = glm::normalize(up + (viewRight * -viewDelta));
	UpdateView();
}

void ViewportCamera::RotateRight() {
	up = glm::normalize(up + (viewRight * viewDelta));
	UpdateView();
}

void ViewportCamera::UpdateView() {
	view.projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f);
	view.viewMatrix = glm::lookAt(eye, center, up);
}

const View& ViewportCamera::GetCameraView() const {
	return view;
}