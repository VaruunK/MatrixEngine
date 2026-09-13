#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/GameObject.hpp"
#include "Core/GameObject/Component/Component.hpp"
#include "Core/Structs/Transform.hpp"
#include "Core/Collection/HashMap.hpp"
#include <typeindex>
#include <type_traits>
#include <optional>
#include <memory>
#include "Entity.reflected.hpp"

class Level;

CLASS()
class MATRIX_API Entity : public GameObject {
	
	REFLECT_CLASS()

public:
	// No copy
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	// No move
	Entity(Entity&&) = delete;
	Entity& operator=(Entity&&) = delete;

	Entity();
	~Entity() = default;

	FUNCTION()
	virtual void Start() override;
	
	FUNCTION()
	virtual void Tick(uint64_t deltaTime) override;

	FUNCTION()
	virtual void DestroyGameObject() override;

	FUNCTION()
	Transform GetTransform() const { return transform; }
	
	FUNCTION()
	glm::vec3 GetLocation() const { return transform.location; }
	
	FUNCTION()
	glm::vec3 GetRotation() const { return transform.rotation; }
	
	FUNCTION()
	glm::vec3 GetScale() const { return transform.scale; }

	FUNCTION()
	Transform SetTransform(Transform transform);
	
	FUNCTION()
	glm::vec3 SetLocation(glm::vec3 location);
	
	FUNCTION()
	glm::vec3 SetRotation(glm::vec3 rotation);
	
	FUNCTION()
	glm::vec3 SetScale(glm::vec3 scale);

	template<typename ComponentType>
	ComponentType* GetComponent() {
    	static_assert(std::is_base_of_v<Component, ComponentType>, "Type does not inherit from Component");
		auto comp = components.find(typeid(ComponentType));
		if (comp == components.end()) {
			return nullptr;
		}
		return static_cast<ComponentType*>(comp->second);
}


	template<typename ComponentType>
	ComponentType* AddComponent() {
		static_assert(std::is_base_of_v<Component, ComponentType>, "Type does not inherit from Component");

		auto comp = new ComponentType(this);
		auto key = std::type_index(typeid(ComponentType));

		components[key] = std::move(comp);

		return comp;
	}

protected:
	FIELD()
	Transform transform;
private:
	void UpdateComponentTransforms();
	
	FIELD()
	HashMap<std::type_index, Component*> components;
};