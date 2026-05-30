#pragma once

#include "Core/GameObject/GameObject.hpp"
#include "Core/GameObject/Entity/Component/Component.hpp"
#include "Transform.hpp"
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <optional>
#include <memory>


class Entity : public GameObject {
public:

	virtual void Start() override;
	virtual void Tick(uint64_t deltaTime) override;
	virtual void DestroyGameObject() override;

	glm::vec3 GetLocation() { return transform.location; }
	glm::vec3 GetRotation() { return transform.rotation; }
	glm::vec3 GetScale() { return transform.scale; }

	glm::vec3 SetLocation(glm::vec3 location) { transform.location = location; return GetLocation(); }
	glm::vec3 SetRotation(glm::vec3 rotation) { transform.rotation = rotation; return GetRotation(); }
	glm::vec3 SetScale(glm::vec3 scale) { transform.scale = scale; return GetScale(); }

	template<typename ComponentType>
	ComponentType* GetComponent() {
		static_assert(std::is_base_of_v<Component, ComponentType>, "Type does not inherit from Component");

		auto it = components.find(std::type_index(typeid(ComponentType)));
		if (it == components.end()) {
			return nullptr;
		}

		return static_cast<ComponentType*>(it->second.get());
	}

	template<typename ComponentType>
	ComponentType* AddComponent() {
		static_assert(std::is_base_of_v<Component, ComponentType>, "Type does not inherit from Component");

		auto comp = std::make_unique<ComponentType>();
		auto key = std::type_index(typeid(ComponentType));

		auto ptr = comp.get();

		components[key] = move(comp);

		return ptr;
	}
protected:
	Transform transform;
private:
	std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
};