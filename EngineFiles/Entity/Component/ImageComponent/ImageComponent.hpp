#pragma once

#include "Entity/Component/Component.hpp"
#include <string>

class SDL_Texture;
class Renderer;

class ImageComponent : public Component {
public:
    void Initialize(Entity* compOwner, const string& filepath);
    void Update(uint64_t deltaTime) override {}
    void DestroyComponent() override;

    SDL_Texture* GetTexture() const;
    const string GetTextureFilePath() const { return textureFilePath; }

    void SetTexture(const string& filepath);

    bool IsVisible() const { return isVisible; }
    void SetVisibility(bool visibility) { isVisible = visibility; }

    float GetScaleX() const { return scaleX; }
    float GetScaleY() const { return scaleY; }
    void SetScale(float x = 1.0f, float y = 1.0f) { scaleX = x; scaleY = y; }

private:
    void Initialize(Entity* compOwner) override;

    bool isVisible = true;
    bool needsChange = false;
    
    string textureFilePath;
    
    Renderer* renderer;

    float scaleX = 1.0f;
    float scaleY = 1.0f;
};