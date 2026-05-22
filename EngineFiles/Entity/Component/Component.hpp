#pragma once
#include "../Entity.hpp"

class Component {
public:

	virtual void Initialize(Entity* compOwner);
	virtual void Update(uint64_t deltaTime) = 0;
	virtual void DestroyComponent() = 0;

	Entity* GetOwner() { return owner; }

	glm::vec3 GetComponentLocation() { return transform.location; }
	glm::vec3 GetComponentRotation() { return transform.rotation; }
	glm::vec3 GetComponentScale() { return transform.scale; }

	glm::vec3 SetComponentLocation(glm::vec3 location);
	glm::vec3 SetComponentRotation(glm::vec3 rotation);
	glm::vec3 SetComponentScale(glm::vec3 scale);
protected:
	Entity* owner;

	Transform transform;
private:
};