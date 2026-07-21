#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/Component/Component.hpp"
#include <string>
#include "SpriteComponent.reflected.hpp"

struct SDL_GPUTexture;
struct SDL_Surface;
struct Texture;

class Entity;

CLASS()
class MATRIX_API SpriteComponent : public Component {
    REFLECTION()
public:
    SpriteComponent(Entity* owner);

    FUNCTION()
    void Start() override;
    FUNCTION()
    void Tick(uint64_t deltaTime) override {}
    FUNCTION()
    void DestroyGameObject() override;

    FUNCTION()
    Texture* GetTexture() { return texture; }
    FUNCTION()
    glm::mat4 GetModelMatrix(float windowAspectRatio);

    FUNCTION()
    void SetTexture(Texture* texture);

    FUNCTION()
    bool IsVisible() const { return isVisible; }
    FUNCTION()
    void SetVisibility(bool visibility) { isVisible = visibility; }

private:
    bool isVisible = true;
    bool needsChange = false;
    
    FIELD()
    Texture* texture;
};