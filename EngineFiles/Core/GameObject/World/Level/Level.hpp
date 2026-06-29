#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/Entity/Entity.hpp"
#include <vector>
#include <memory>

class World;

struct Transform;

class MATRIX_API Level {
public:
	Level(World* world);

	// No copy
	Level(const Level&) = delete;
	Level& operator=(const Level&) = delete;

	// No move
	Level(Level&&) = delete;
	Level& operator=(Level&&) = delete;
	
	void Load();
	void Unload();

	template<typename ClassType>
	ClassType* AddEntityToLevel(Transform& transform) {
		static_assert(std::is_base_of_v<Entity, ClassType>, "ClassType does not inherit from Entity");
		auto newEntity = new ClassType(this);
		newEntity->SetTransform(transform);
		entities.push_back(newEntity);
		return newEntity;
	}

	template<typename ClassType>
	ClassType* SpawnFromClass(Transform& transform) {
		auto newEntity = AddEntityToLevel<ClassType>(transform);
		newEntity->Start();
		return newEntity;
	}
	
	World* GetWorld() { return world; }

	const std::vector<Entity*>& GetAllEntities() { return entities; }
protected:
private:
	World* world;
	std::vector<Entity*> entities;
};