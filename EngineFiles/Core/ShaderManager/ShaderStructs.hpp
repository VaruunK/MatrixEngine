#pragma once

#include <string>
#include <cstdint>

using namespace std;

class SDL_GPUShaderStage;

struct ShaderInfo {
    string sourcePath;
    string compiledPath;
    string entryPoint;
    SDL_GPUShaderStage stage;
    uint64_t sourceTimestamp;
    uint64_t compiledTimestamp;
};

enum class ShaderFeatures {
    None = 0,
    Lighting = 1 << 0,
    Shadows = 1 << 1,
    NormalMapping = 1 << 2,
};