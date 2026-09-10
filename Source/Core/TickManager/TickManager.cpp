#include "TickManager.hpp"
#include "Core/GameObject/GameObject.hpp"

void TickManager::Tick(float deltaTime) {
	for (auto& go : tickingGameObjects) {
		go->Tick(deltaTime);
	}
}

void TickManager::RegisterToTick(GameObject* gameObject)
{
	tickingGameObjects.push_back(gameObject);
}

void TickManager::DeregisterFromTick(GameObject* gameObject)
{

}
