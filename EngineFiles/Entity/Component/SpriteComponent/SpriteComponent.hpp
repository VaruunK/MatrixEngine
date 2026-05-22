#pragma once

#include "Entity/Component/Component.hpp"
#include <string>
#include <glm/glm.hpp>

struct SDL_GPUTexture;
struct SDL_Surface;

struct Texture {
    SDL_GPUTexture* texture;
    SDL_Surface* data;
};

class SpriteComponent : public Component {
public:
    void Initialize(Entity* compOwner) override;
    void Update(uint64_t deltaTime) override {}
    void DestroyComponent() override;

    const Texture* GetTexture() { return texture; }
    const std::string GetTextureFilePath() { return textureFilePath; }
    const glm::mat4 GetModelMatrix(float windowAspectRatio);

    void SetTexture(const std::string& filepath);

    bool IsVisible() const { return isVisible; }
    void SetVisibility(bool visibility) { isVisible = visibility; }

private:
    
    Texture* CreateTexture(const std::string& texturePath);

    bool isVisible = true;
    bool needsChange = false;
    
    std::string textureFilePath;
    Texture* texture;
};