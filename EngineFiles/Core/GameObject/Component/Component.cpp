#include "Component.hpp"
#include "Core/GameObject/Entity/Entity.hpp"

Component::Component(Entity* owner) : GameObject() {
	this->owner = owner;
}

void Component::Start() {
	GameObject::Start();
}

void Component::Tick(uint64_t deltaTime) {
	GameObject::Tick(deltaTime);
}

void Component::DestroyGameObject() {
	GameObject::DestroyGameObject();
}

glm::vec3 Component::GetComponentLocation() {
	return transform.location;
}

glm::vec3 Component::GetComponentRotation() {
	return transform.rotation;
}

glm::vec3 Component::GetComponentScale() {
	return transform.scale;
}

glm::vec3 Component::SetComponentLocation(glm::vec3 location) {
	transform.location = location;
	return GetComponentLocation();
}

glm::vec3 Component::SetComponentRotation(glm::vec3 rotation) {
	auto normalize = [](float angle) {
		angle = fmod(angle, 360.0f);
		if (angle < 0.0f) {
			angle += 360.0f;
		}
		return angle;
	};
	transform.rotation.x = normalize(rotation.x);
	transform.rotation.y = normalize(rotation.y);
	transform.rotation.z = normalize(rotation.z);
	return GetComponentRotation();
}

glm::vec3 Component::SetComponentScale(glm::vec3 scale) {
	auto validate = [](float scalar) {
		if (scalar < 0.0f) {
			return 1.0f;
		}
		return scalar;
	};
	transform.scale.x = validate(scale.x);
	transform.scale.y = validate(scale.y);
	transform.scale.z = validate(scale.z);
	return GetComponentScale();
}