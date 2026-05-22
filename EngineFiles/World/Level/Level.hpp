#pragma once

#include "Core/GameObject/GameObject.hpp"
#include "Entity/Entity.hpp"
#include <vector>
#include <memory>

class Level : public GameObject {
public:
	Level();
	
	void Load();
	void Unload();

	void Start() override;
	void DestroyGameObject() override;

	void Tick(uint64_t deltaTime, const std::vector<GameObject*> &tickingGameObjects);

	template<typename ClassType>
	ClassType* AddEntityToLevel(float x = 0.0f, float y = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f) {
		static_assert(std::is_base_of_v<Entity, ClassType>, "ClassType does not inherit from Entity");
		auto newEntity = std::make_unique<ClassType>();
		ClassType* raw = newEntity.get();
		entities.push_back(move(newEntity));
		return raw;
	}

	template<typename ClassType>
	ClassType* SpawnFromClass(float x = 0.0f, float y = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f) {
		static_assert(std::is_base_of_v<Entity, ClassType>, "ClassType does not inherit from Entity");
		auto newEntity = std::make_unique<ClassType>();
		ClassType *raw = newEntity.get();
		entities.push_back(std::move(newEntity));
		raw->Start();
		return raw;
	}

	const std::vector<std::unique_ptr<GameObject>>& GetAllEntities() { return entities; }
protected:
private:
	void Tick(uint64_t deltaTime) override;
	std::vector<std::unique_ptr<GameObject>> entities;
};