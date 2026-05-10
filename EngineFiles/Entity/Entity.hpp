#pragma once

#include "Core/GameObject/GameObject.hpp"
#include "Core/Math/vec2.h"
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

	vec2f GetPosition() { return position; }
	float GetXPos() { return position.x; }
	float GetYPos() { return position.y; }

	void SetPosition(vec2f newPosition) { position = newPosition; }

	template<typename ComponentType>
	ComponentType* GetComponent() {
		static_assert(is_base_of_v<Component, ComponentType>, "Type does not inherit from Component");

		auto it = components.find(type_index(typeid(ComponentType)));
		if (it == components.end()) {
			return nullptr;
		}

		return static_cast<ComponentType*>(it->second.get());
	}

	template<typename ComponentType>
	ComponentType* AddComponent() {
		static_assert(is_base_of_v<Component, ComponentType>, "Type does not inherit from Component");

		auto comp = make_unique<ComponentType>();
		auto key = type_index(typeid(ComponentType));

		auto ptr = comp.get();

		components[key] = move(comp);

		return ptr;
	}
protected:
private:
	unordered_map<type_index, unique_ptr<Component>> components;

	vec2f position = vec2f(0.0f, 0.0f);
};