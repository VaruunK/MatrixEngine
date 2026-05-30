#include "Entity.hpp"
#include "Component/Component.hpp"

void Entity::Start()
{
	GameObject::Start();
}

void Entity::Tick(uint64_t deltaTime)
{
	GameObject::Tick(deltaTime);
	for (auto& [type, component] : components) {
		component->Update(deltaTime);
	}
}

void Entity::DestroyGameObject()
{
	GameObject::DestroyGameObject();
}