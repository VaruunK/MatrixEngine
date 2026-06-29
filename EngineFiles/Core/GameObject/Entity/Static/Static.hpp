#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/Entity/Entity.hpp"
#include <memory>

class ImageComponent;
class MovementComponent;

class MATRIX_API Static : public Entity {
public:

	Static(Level* level);

	void Start() override;
	void Tick(uint64_t) override;
	void DestroyGameObject() override;
protected:
private:
	ImageComponent* imageComponent;
	MovementComponent* movementComponent;
};