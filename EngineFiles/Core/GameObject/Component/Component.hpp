#pragma once

#include "Core/GameObject/GameObject.hpp"
#include "Core/Structs/Transform.hpp"
#include <glm/glm.hpp>

class Entity;

class Component : public GameObject {
public:

	virtual void Initialize(Entity* compOwner);
	virtual void Update(uint64_t deltaTime) = 0;
	virtual void DestroyComponent() = 0;

	GameObject* GetOwner() { return owner; }

	glm::vec3 GetComponentLocation();
	glm::vec3 GetComponentRotation();
	glm::vec3 GetComponentScale();

	glm::vec3 SetComponentLocation(glm::vec3 location);
	glm::vec3 SetComponentRotation(glm::vec3 rotation);
	glm::vec3 SetComponentScale(glm::vec3 scale);
protected:
	GameObject* owner;

	Transform transform;
private:
};