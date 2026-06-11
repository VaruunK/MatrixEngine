#pragma once

#include <vector>
#include <memory>
#include "Core/GameObject/Entity/Entity.hpp"

class World;

class Level {
public:
	Level(World* world);
	
	void Load();
	void Unload();

	template<typename ClassType>
	ClassType* AddEntityToLevel() {
		static_assert(std::is_base_of_v<Entity, ClassType>, "ClassType does not inherit from Entity");
		auto newEntity = new ClassType(this);
		
		entities.push_back(newEntity);
		return newEntity;
	}

	template<typename ClassType>
	ClassType* SpawnFromClass() {
		auto newEntity = AddEntityToLevel<ClassType>();
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