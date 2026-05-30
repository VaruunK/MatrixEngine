#include "static.hpp"
#include "Entity/Component/ImageComponent/ImageComponent.hpp"
#include "Entity/Component/MovementComponent/MovementComponent.hpp"
#include <iostream>

Static::Static() : Entity() {
	canTick = false;
	
	imageComponent = AddComponent<ImageComponent>();
	imageComponent->Initialize(this, "");

	movementComponent = AddComponent<MovementComponent>();
	movementComponent->Initialize(this, E_Stationary);
}

void Static::Start()
{
	Entity::Start();
}

void Static::Tick(uint64_t deltaTime)
{
	Entity::Tick(deltaTime);
}

void Static::DestroyGameObject()
{
	Entity::DestroyGameObject();
}