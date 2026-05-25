#pragma once

#include "Entity/Component/Component.hpp"
#include <string>
#include <glm/glm.hpp>

struct SDL_GPUTexture;
struct SDL_Surface;
struct Texture;

class SpriteComponent : public Component {
public:
    void Initialize(Entity* compOwner) override;
    void Update(uint64_t deltaTime) override {}
    void DestroyComponent() override;

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