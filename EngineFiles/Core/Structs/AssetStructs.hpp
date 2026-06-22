#pragma once

#include "Core/Structs/RenderStructs.hpp"
#include <SDL3/SDL.h>
#include <string>
#include <vector>

struct Texture {
    SDL_GPUTexture* texture = nullptr;
    SDL_Surface* data = nullptr;
    std::string texturePath;
};

struct Mesh {
    Texture* texture = nullptr;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};