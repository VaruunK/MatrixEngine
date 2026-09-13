#include "GameObject.hpp"

#include "Core/Statics/GameStatics.hpp"
#include "Core/GameObject/World/Level/Level.hpp"

static long long IDCounter = 0;

GameObject::GameObject() {
	canTick = true;
	hasStarted = false;
	id = IDCounter++;
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