#pragma once

#include "Core/GameObject/GameObject.hpp"
#include "Core/GameObject/Component/Component.hpp"
#include "Core/Structs/Transform.hpp"
#include "Core/Editor/Viewport/SelectProxy/SelectProxy.hpp"
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <optional>
#include <memory>

class Level;

class Entity : public GameObject {
public:

	Entity(Level* level);
	~Entity() = default;

	virtual void Start() override;
	virtual void Tick(uint64_t deltaTime) override;
	virtual void DestroyGameObject() override;

	Transform GetTransform() const { return transform; }
	glm::vec3 GetLocation() const { return transform.location; }
	glm::vec3 GetRotation() const { return transform.rotation; }
	glm::vec3 GetScale() const { return transform.scale; }

	Transform SetTransform(Transform transform);
	glm::vec3 SetLocation(glm::vec3 location);
	glm::vec3 SetRotation(glm::vec3 rotation);
	glm::vec3 SetScale(glm::vec3 scale);

	Level* GetLevel() { return currentLevel; }

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

		auto comp = std::make_unique<ComponentType>(this);
		auto key = std::type_index(typeid(ComponentType));

		auto ptr = comp.get();

		components[key] = move(comp);

		return ptr;
	}

	SelectProxy selectProxy;
protected:
	Transform transform;
private:
	void UpdateComponentTransforms();
	std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
	Level* currentLevel;
};