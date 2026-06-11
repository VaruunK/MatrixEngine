#pragma once

#include "Core/GameObject/Component/Component.hpp"
#include <string>

struct SDL_GPUTexture;
struct SDL_Surface;
struct Texture;

class Entity;

class SpriteComponent : public Component {
public:
    SpriteComponent(Entity* owner);

    void Start() override;
    void Tick(uint64_t deltaTime) override {}
    void DestroyGameObject() override;

    const Texture* GetTexture() { return texture; }
    const glm::mat4 GetModelMatrix(float windowAspectRatio);

    void SetTexture(Texture* texture);

    bool IsVisible() const { return isVisible; }
    void SetVisibility(bool visibility) { isVisible = visibility; }

private:
    bool isVisible = true;
    bool needsChange = false;
    
    Texture* texture;
};