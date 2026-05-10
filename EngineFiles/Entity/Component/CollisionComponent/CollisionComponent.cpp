#include "CollisionComponent.hpp"
#include "Entity/Component/Component.hpp"
#include <SDL3/SDL.h>

void CollisionComponent::Initialize(Entity* compOwner) {
	Component::Initialize(compOwner);


}

void CollisionComponent::Update(uint64_t deltaTime) {

}

void CollisionComponent::DestroyComponent()
{
}

//const SDL_FRect CollisionComponent::GetBounds() {
//	return SDL_FRect{ owner->GetXPos(), owner->GetYPos(), width * scaleX, height * scaleY};
//}