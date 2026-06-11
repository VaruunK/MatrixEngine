#include "Entity.hpp"
#include "Core/GameObject/Component/Component.hpp"

Entity::Entity(Level* level) {
	this->currentLevel = level;
}

void Entity::Start() {
	GameObject::Start();
}

void Entity::Tick(uint64_t deltaTime) {
	GameObject::Tick(deltaTime);
	for (auto& [type, component] : components) {
		component->Tick(deltaTime);
	}
}

void Entity::DestroyGameObject()
{
	GameObject::DestroyGameObject();
}