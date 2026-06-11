#pragma once

#include "Core/GameObject/GameObject.hpp"
#include "Core/Structs/Transform.hpp"
#include <glm/glm.hpp>

class Entity;

class Component : public GameObject {
public:

	Component(Entity* owner);

	virtual void Start() override;
	virtual void Tick(uint64_t deltaTime) override;
	virtual void DestroyGameObject() override;

	Entity* GetOwner() { return owner; }

	glm::vec3 GetComponentLocation();
	glm::vec3 GetComponentRotation();
	glm::vec3 GetComponentScale();

	glm::vec3 SetComponentLocation(glm::vec3 location);
	glm::vec3 SetComponentRotation(glm::vec3 rotation);
	glm::vec3 SetComponentScale(glm::vec3 scale);
protected:
	Entity* owner;

	Transform transform;
private:
};