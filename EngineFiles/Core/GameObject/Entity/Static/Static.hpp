#include "GameObject/Entity/Entity.hpp"
#include <memory>

class ImageComponent;
class MovementComponent;

class Static : public Entity {
public:

	Static();

	void Start() override;
	void Tick(uint64_t) override;
	void DestroyGameObject() override;
protected:
private:
	ImageComponent* imageComponent;
	MovementComponent* movementComponent;
};