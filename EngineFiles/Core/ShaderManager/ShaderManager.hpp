#pragma once

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>

struct Appstate;

struct CompiledShader {
    std::vector<uint8_t> bytecode;
    uint64_t sourceHash = 0;

    uint32_t num_samplers = 0;
    uint32_t num_storage_textures = 0;
    uint32_t num_storage_buffers = 0;
    uint32_t num_uniform_buffers = 0;
};

struct ShaderOptions {
    uint32_t num_samplers = 0;
    uint32_t num_storage_textures = 0;
    uint32_t num_storage_buffers = 0;
    uint32_t num_uniform_buffers = 0;
};

class ShaderManager {
public:

    static ShaderManager& Get() {
        return *instance;
    }

    ~ShaderManager() {
        if (!isShutdown) {
            Shutdown();
        }
    }

    static void Init(Appstate& appstate) {
        instance.reset(new ShaderManager(appstate));
    }
    
    static void Shutdown() {
        if (instance) {
            instance->ShutdownInternal();
            instance.reset();
        }
    }

    SDL_GPUShader* LoadShader(const std::string& shaderPath, const ShaderOptions *options, const std::string& entryPoint = "main");
    SDL_GPUShader* LoadShader(const std::string& shaderPath, const std::string& entryPoint = "main");
    //SDL_GPUComputePipeline* CreateComputePipelineFromShader(SDL_GPUDevice* device, const string& shaderFilename, const string& entrypoint = "main", SDL_GPUComputePipelineCreateInfo* createInfo);
    SDL_GPUShader* GetShader(const std::string& name);

    void ClearCache();
    
private:
    ShaderManager(Appstate& appstate);
    void ShutdownInternal();

    std::filesystem::path cacheDir;

    Appstate& appstate;

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

    static inline std::unique_ptr<ShaderManager> instance = nullptr;

    bool isShutdown = false;
};