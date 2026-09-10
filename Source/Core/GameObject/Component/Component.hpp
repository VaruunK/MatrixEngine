#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/GameObject.hpp"
#include "Core/Structs/Transform.hpp"
#include <glm/glm.hpp>
#include "Component.reflected.hpp"

class Entity;

CLASS()
class MATRIX_API Component : public GameObject {
	friend class Entity;

	REFLECT_CLASS()
public:

	Component(Entity* owner);

	// No copy
	Component(const Component&) = delete;
	Component& operator=(const Component&) = delete;

	// No move
	Component(Component&&) = delete;
	Component& operator=(Component&&) = delete;

	FUNCTION()
	virtual void Start() override;
	
	FUNCTION()
	virtual void Tick(uint64_t deltaTime) override;
	
	FUNCTION()
	virtual void DestroyGameObject() override;

	FUNCTION()
	Entity* GetOwner() { return owner; }

	FUNCTION()
	Transform GetComponentTransform();
	FUNCTION()
	glm::vec3 GetComponentLocation();
	FUNCTION()
	glm::vec3 GetComponentRotation();
	FUNCTION()
	glm::vec3 GetComponentScale();

	FUNCTION()
	Transform GetLocalComponentTransform() const { return localTransform; };
	FUNCTION()
	glm::vec3 GetLocalComponentLocation();
	FUNCTION()
	glm::vec3 GetLocalComponentRotation();
	FUNCTION()
	glm::vec3 GetLocalComponentScale();

	FUNCTION()
	Transform SetLocalComponentTransform(Transform& transform);
	FUNCTION()
	glm::vec3 SetLocalComponentLocation(glm::vec3& location);
	FUNCTION()
	glm::vec3 SetLocalComponentRotation(glm::vec3& rotation);
	FUNCTION()
	glm::vec3 SetLocalComponentScale(glm::vec3& scale);
protected:
	Entity* owner;

	Transform localTransform;
	Transform transform;
private:
	void RecalculateGlobalTransform();
};