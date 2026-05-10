#include "GameObject.hpp"
#include "../../../EngineFiles/Engine.hpp"

GameObject::GameObject() {
	canTick = true;
	hasStarted = false;
}

void GameObject::Start() {
	if (hasStarted) {
		return;
	}
	hasStarted = true;
	if (canTick) {
		Engine::GetEngine().GetTickManager().RegisterToTick(this);
	}
}

void GameObject::Tick(uint64_t deltaTime) {

}

void GameObject::DestroyGameObject() {

}

World& GameObject::GetWorld() {
	return Engine::GetEngine().GetWorld();
}