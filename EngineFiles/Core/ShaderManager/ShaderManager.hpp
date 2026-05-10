#pragma once

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <functional>

struct SDLGPUShaderDeleter {
    SDL_GPUDevice* device;
    void operator()(SDL_GPUShader* ptr) const {
        if (ptr && device) {
            SDL_ReleaseGPUShader(device, ptr);
        }
    }
};

struct CompiledShader {
    std::vector<uint8_t> bytecode;
    uint64_t sourceHash;

    uint32_t num_samplers;
    uint32_t num_storage_textures;
    uint32_t num_storage_buffers;
    uint32_t num_uniform_buffers;
};

class ShaderManager {
public:

    ShaderManager(SDL_GPUDevice* device);

    void Initialize();
    void Shutdown();

    SDL_GPUShader* LoadShader(const std::string& shaderPath, const std::string& entryPoint = "main");
    //SDL_GPUComputePipeline* CreateComputePipelineFromShader(SDL_GPUDevice* device, const string& shaderFilename, const string& entrypoint = "main", SDL_GPUComputePipelineCreateInfo* createInfo);
    SDL_GPUShader* GetShader(const std::string& name);

    void ClearCache();

private:
    SDL_GPUDevice* device;
    std::filesystem::path cacheDir;

    std::unordered_map<std::string, SDL_GPUShader*> shaderCache;
    std::unordered_map<std::string, CompiledShader> memoryCache;

    SDL_GPUShader* CompileShader(const std::string& sourcePath, SDL_GPUShaderStage stage,
        const std::string& entryPoint, CompiledShader& outBytecode);

    SDL_GPUShader* CreateShaderFromSPIRV(const CompiledShader& compiled,
        SDL_GPUShaderStage stage,
        const std::string& entryPoint);

    std::string GetCachePath(const std::string& shaderPath, SDL_GPUShaderStage stage);
    void SaveToDiskCache(const std::string& cachePath, const CompiledShader& shader);
    bool LoadFromDiskCache(const std::string& cachePath, CompiledShader& outShader);

    std::vector<uint8_t> ReadFile(const std::string& filepath);
    uint64_t HashFile(const std::string& filepath);
    SDL_GPUShaderStage GetTargetStage(const std::string& shaderFileName);
    SDL_ShaderCross_ShaderStage ConvertStage(SDL_GPUShaderStage stage);
};