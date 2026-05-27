#pragma once

#include "Core/Render/RenderStructs.hpp"
#include <SDL3/SDL.h>
#include <string>
#include <vector>

struct Texture {
    SDL_GPUTexture* texture;
    SDL_Surface* data;
    std::string texturePath;
};

struct Mesh {
    Texture* texture;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};