#include "GameObject.hpp"

GameObject::GameObject() {
	canTick = true;
	hasStarted = false;
	// Engine::GetEngine().GetWorld();
}

void GameObject::Start() {
	if (hasStarted) {
		return;
	}
	hasStarted = true;
	if (canTick) {
		// Engine::GetEngine().GetTickManager().RegisterToTick(this);
	}
}

void GameObject::Tick(uint64_t deltaTime) {

}

void GameObject::DestroyGameObject() {

}

// need a get world function