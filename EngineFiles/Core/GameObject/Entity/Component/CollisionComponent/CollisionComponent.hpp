#pragma once

#include "Entity/Component/Component.hpp"
#include "Core/PhysicsManager/CollisionStructs.hpp"
#include "Engine.hpp"
#include <optional>

class SDL_FRect;

class CollisionComponent : public Component {
public:
	void Initialize(Entity* compOwner) override;
	void Update(uint64_t deltaTime) override;
	void DestroyComponent() override;

	template<typename EntityType>
	void AddCollisionResponse(CollisionType response) {
		static_assert(is_base_of_v<Entity, EntityType>, "Type does not inherit from Entity");

		auto typeIdx = type_index(typeid(EntityType));

		CollisionResponse collisionResponse{};
		collisionResponse.type = typeIdx;
		collisionResponse.response = response;

		Engine::GetEngine().GetPhysicsManager().SetCollisionResponse(*handle, collisionResponse);
	}

protected:
private:
	bool renderBounds;

	float boxWidth = 1.0f;
	float boxHeight = 1.0f;

	SDL_FRect collisionBox;
	optional<CollisionHandle> handle;
};