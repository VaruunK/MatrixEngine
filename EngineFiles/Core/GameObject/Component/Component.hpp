#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/GameObject.hpp"
#include "Core/Structs/Transform.hpp"
#include <glm/glm.hpp>

class Entity;

class MATRIX_API Component : public GameObject {
	friend class Entity;
public:

	Component(Entity* owner);

	// No copy
	Component(const Component&) = delete;
	Component& operator=(const Component&) = delete;

	// No move
	Component(Component&&) = delete;
	Component& operator=(Component&&) = delete;

	virtual void Start() override;
	virtual void Tick(uint64_t deltaTime) override;
	virtual void DestroyGameObject() override;

	Entity* GetOwner() { return owner; }

	Transform GetComponentTransform();
	glm::vec3 GetComponentLocation();
	glm::vec3 GetComponentRotation();
	glm::vec3 GetComponentScale();

	Transform GetLocalComponentTransform() const { return localTransform; };
	glm::vec3 GetLocalComponentLocation();
	glm::vec3 GetLocalComponentRotation();
	glm::vec3 GetLocalComponentScale();

	Transform SetLocalComponentTransform(Transform& transform);
	glm::vec3 SetLocalComponentLocation(glm::vec3& location);
	glm::vec3 SetLocalComponentRotation(glm::vec3& rotation);
	glm::vec3 SetLocalComponentScale(glm::vec3& scale);
protected:
	Entity* owner;

	Transform localTransform;
	Transform transform;
private:
	void RecalculateGlobalTransform();
};