#include "Entity.hpp"
#include "Core/GameObject/Component/Component.hpp"

Entity::Entity(Level* level) {
	this->currentLevel = level;
}

void Entity::Start() {
	GameObject::Start();
}

void Entity::Tick(uint64_t deltaTime) {
	GameObject::Tick(deltaTime);
	for (auto& [type, component] : components) {
		component->Tick(deltaTime);
	}
}

void Entity::DestroyGameObject() {
	GameObject::DestroyGameObject();
}

Transform Entity::SetTransform(Transform transform) {
	SetLocation(transform.location);
	SetRotation(transform.rotation);
	SetScale(transform.scale);
	return GetTransform();
}

glm::vec3 Entity::SetLocation(glm::vec3 location) {
	transform.location = location;
	UpdateComponentTransforms();
	return GetLocation();
}

glm::vec3 Entity::SetRotation(glm::vec3 rotation) {
	auto normalize = [](float angle) {
		angle = fmod(angle, 360.0f);
		if (angle < 0.0f) angle += 360.0f;
		return angle;
		};
	transform.rotation.x = normalize(rotation.x);
	transform.rotation.y = normalize(rotation.y);
	transform.rotation.z = normalize(rotation.z);
	UpdateComponentTransforms();
	return GetRotation();
}

glm::vec3 Entity::SetScale(glm::vec3 scale) {
	auto validate = [](float scalar) {
		if (scalar < 0.0f) return 1.0f;
		return scalar;
		};
	transform.scale.x = validate(scale.x);
	transform.scale.y = validate(scale.y);
	transform.scale.z = validate(scale.z);
	UpdateComponentTransforms();
	return GetScale();
}

void Entity::UpdateComponentTransforms() {
	for (auto& [type, component] : components) {
		component->RecalculateGlobalTransform();
	}
}
