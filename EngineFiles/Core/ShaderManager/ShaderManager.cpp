#include "ShaderManager.hpp"
#include <iostream>

ShaderManager::ShaderManager(SDL_GPUDevice* device) : device(device) {
    if (!device) {
        throw std::runtime_error("Couldn't get Device");
    }

    cacheDir = "shaderCache";
    std::filesystem::create_directories(cacheDir);
}

void ShaderManager::Initialize() {

}

void ShaderManager::Shutdown() {
    for (auto& [key, shader] : shaderCache) {
        SDL_ReleaseGPUShader(device, shader);
    }
    shaderCache.clear();
}

void ShaderManager::ClearCache() {
    memoryCache.clear();
    std::filesystem::remove_all(cacheDir);
    std::filesystem::create_directories(cacheDir);
}


//SDL_GPUComputePipeline* ShaderManager::CreateComputePipelineFromShader(SDL_GPUDevice *device, 
//    const string& sourcePath, const string& entrypoint, SDL_GPUComputePipelineCreateInfo *createInfo) {
//    
//    SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
//    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV;
//
//    size_t codeSize;
//    void* code = SDL_LoadFile(sourcePath.c_str(), &codeSize);
//    if (code == nullptr) {
//        SDL_Log("Failed to load compute shader from disk! %s", sourcePath);
//        return nullptr;
//    }
//
//    // Make a copy of the create data, then overwrite the parts we need
//    SDL_GPUComputePipelineCreateInfo newCreateInfo = *createInfo;
//    newCreateInfo.code = static_cast<uint8_t*>(code);
//    newCreateInfo.code_size = codeSize;
//    newCreateInfo.entrypoint = entrypoint.c_str();
//    newCreateInfo.format = format;
//
//    SDL_GPUComputePipeline* pipeline = SDL_CreateGPUComputePipeline(device, &newCreateInfo);
//    if (pipeline == nullptr) {
//        SDL_Log("Failed to create compute pipeline!");
//        SDL_free(code);
//        return nullptr;
//    }
//
//    SDL_free(code);
//    return pipeline;
//}

SDL_GPUShader* ShaderManager::LoadShader(const std::string& shaderPath, const std::string& entryPoint) {
    // need load shader to accept arguments to modify compiledshader
    // or something similar
    
    std::string cacheKey = shaderPath;
    
    // Check if we already have the GPU shader
    if (shaderCache.contains(cacheKey)) {
        return shaderCache[cacheKey];
    }

    SDL_GPUShaderStage stage = GetTargetStage(shaderPath);
    CompiledShader compiled;
    SDL_GPUShader* shader = nullptr;
    bool needsCompile = false;

    // Check memory cache
    if (memoryCache.contains(cacheKey)) {
        uint64_t currentHash = HashFile(shaderPath);
        if (memoryCache[cacheKey].sourceHash == currentHash) {
            compiled = memoryCache[cacheKey];
        }
        else {
            needsCompile = true;
        }
    }
    else {
        // Check disk cache
        std::string diskCachePath = GetCachePath(shaderPath, stage);
        if (LoadFromDiskCache(diskCachePath, compiled)) {
            uint64_t currentHash = HashFile(shaderPath);
            if (compiled.sourceHash != currentHash) {
                needsCompile = true;
            }
        }
        else {
            needsCompile = true;
        }
    }

    if (needsCompile) {
        shader = CompileShader(shaderPath, stage, entryPoint, compiled);
        memoryCache[cacheKey] = compiled;
        SaveToDiskCache(GetCachePath(shaderPath, stage), compiled);
    }
    else {
        // Create shader from cached SPIRV
        shader = CreateShaderFromSPIRV(compiled, stage, entryPoint);
    }

    shaderCache[cacheKey] = shader;
    return shader;
}

SDL_GPUShader* ShaderManager::LoadShader(const std::string& shaderPath, const ShaderOptions *options, const std::string& entryPoint) {
    std::string cacheKey = shaderPath;

    // Check if we already have the GPU shader
    if (shaderCache.contains(cacheKey)) {
        return shaderCache[cacheKey];
    }

    SDL_GPUShaderStage stage = GetTargetStage(shaderPath);
    CompiledShader compiled;
    compiled.num_samplers = options->num_samplers;
    compiled.num_storage_buffers = options->num_storage_buffers;
    compiled.num_storage_textures = options->num_storage_textures;
    compiled.num_uniform_buffers = options->num_uniform_buffers;
    SDL_GPUShader* shader = nullptr;
    bool needsCompile = false;

    // Check memory cache
    if (memoryCache.contains(cacheKey)) {
        uint64_t currentHash = HashFile(shaderPath);
        if (memoryCache[cacheKey].sourceHash == currentHash) {
            compiled = memoryCache[cacheKey];
        }
        else {
            needsCompile = true;
        }
    }
    else {
        // Check disk cache
        std::string diskCachePath = GetCachePath(shaderPath, stage);
        if (LoadFromDiskCache(diskCachePath, compiled)) {
            uint64_t currentHash = HashFile(shaderPath);
            if (compiled.sourceHash != currentHash) {
                needsCompile = true;
            }
        }
        else {
            needsCompile = true;
        }
    }

    if (needsCompile) {
        shader = CompileShader(shaderPath, stage, entryPoint, compiled);
        memoryCache[cacheKey] = compiled;
        SaveToDiskCache(GetCachePath(shaderPath, stage), compiled);
    }
    else {
        // Create shader from cached SPIRV
        shader = CreateShaderFromSPIRV(compiled, stage, entryPoint);
    }

    shaderCache[cacheKey] = shader;
    return shader;
}

SDL_GPUShader* ShaderManager::CreateShaderFromSPIRV(const CompiledShader& compiled, SDL_GPUShaderStage stage, const std::string& entryPoint) {
    SDL_ShaderCross_SPIRV_Info spirvInfo = {};
    spirvInfo.bytecode = compiled.bytecode.data();
    spirvInfo.bytecode_size = compiled.bytecode.size();
    spirvInfo.entrypoint = entryPoint.c_str();
    spirvInfo.shader_stage = ConvertStage(stage);
    spirvInfo.props = 0;

    SDL_ShaderCross_GraphicsShaderResourceInfo resourceInfo = {};
    resourceInfo.num_samplers = compiled.num_samplers;
    resourceInfo.num_storage_textures = compiled.num_storage_textures;
    resourceInfo.num_storage_buffers = compiled.num_storage_buffers;
    resourceInfo.num_uniform_buffers = compiled.num_uniform_buffers;

    SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
        device,
        &spirvInfo,
        &resourceInfo,
        0
    );

    if (!shader) {
        throw std::runtime_error("Failed to create GPU shader from cached SPIRV");
    }

    return shader;
}

SDL_GPUShader* ShaderManager::CompileShader(const std::string& sourcePath, SDL_GPUShaderStage stage,
    const std::string& entryPoint, CompiledShader& outBytecode) {
    size_t sourceSize = 0;
    void* sourceData = SDL_LoadFile(sourcePath.c_str(), &sourceSize);
    if (!sourceData) {
        throw std::runtime_error("Failed to load shader file: " + sourcePath);
    }

    // Extract the directory from the source path for includes
    std::filesystem::path shaderPath(sourcePath);
    std::filesystem::path includeDir = shaderPath.parent_path();
    std::string includeDirStr = includeDir.string();

    SDL_ShaderCross_HLSL_Info hlslInfo = {};
    hlslInfo.source = static_cast<const char*>(sourceData);
    hlslInfo.entrypoint = entryPoint.c_str();
    hlslInfo.include_dir = includeDirStr.empty() ? nullptr : includeDirStr.c_str();
    hlslInfo.defines = nullptr;
    hlslInfo.shader_stage = ConvertStage(stage);
    hlslInfo.props = 0;

    size_t spirvSize = 0;
    void* spirvBytecode = nullptr;

    spirvBytecode = SDL_ShaderCross_CompileSPIRVFromHLSL(
        &hlslInfo,
        &spirvSize
    );

    SDL_free(sourceData);

    if (!spirvBytecode) {
        const char* err = SDL_GetError();
        SDL_Log("ShaderCross error: %s", err ? err : "no error message");
        throw std::runtime_error("Failed to compile HLSL to SPIRV: " + sourcePath);
    }

    SDL_Log("Successfully compiled to SPIRV: %zu bytes", spirvSize);

    SDL_ShaderCross_GraphicsShaderMetadata* metadata =
        SDL_ShaderCross_ReflectGraphicsSPIRV(
            static_cast<const Uint8*>(spirvBytecode),
            spirvSize,
            0
        );

    if (!metadata) {
        SDL_free(spirvBytecode);
        throw std::runtime_error("Failed to reflect SPIRV: " + sourcePath);
    }

    SDL_ShaderCross_SPIRV_Info spirvInfo = {};
    spirvInfo.bytecode = static_cast<const Uint8*>(spirvBytecode);
    spirvInfo.bytecode_size = spirvSize;
    spirvInfo.entrypoint = entryPoint.c_str();
    spirvInfo.shader_stage = ConvertStage(stage);
    spirvInfo.props = 0;

    SDL_GPUShader* shader = SDL_ShaderCross_CompileGraphicsShaderFromSPIRV(
        device,
        &spirvInfo,
        &metadata->resource_info,
        0
    );

    if (!shader) {
        SDL_free(metadata);
        SDL_free(spirvBytecode);
        throw std::runtime_error("Failed to create GPU shader: " + sourcePath);
    }

    // Store bytecode for caching
    outBytecode.bytecode.assign(
        static_cast<uint8_t*>(spirvBytecode),
        static_cast<uint8_t*>(spirvBytecode) + spirvSize
    );

    outBytecode.sourceHash = HashFile(sourcePath);
    outBytecode.num_samplers = metadata->resource_info.num_samplers;
    outBytecode.num_storage_textures = metadata->resource_info.num_storage_textures;
    outBytecode.num_storage_buffers = metadata->resource_info.num_storage_buffers;
    outBytecode.num_uniform_buffers = metadata->resource_info.num_uniform_buffers;

    SDL_free(metadata);
    SDL_free(spirvBytecode);

    return shader;
}

std::string ShaderManager::GetCachePath(const std::string& shaderPath, SDL_GPUShaderStage stage) {
    std::filesystem::path p(shaderPath);
    std::string filename = p.stem().string();

    std::string stageStr;
    switch (stage) {
    case SDL_GPU_SHADERSTAGE_VERTEX:
        stageStr = "vert";
        break;
    case SDL_GPU_SHADERSTAGE_FRAGMENT:
        stageStr = "frag";
        break;
    default:
        stageStr = "unknown";
        break;
    }

    return (cacheDir / (filename + ".cache")).string();
}

void ShaderManager::SaveToDiskCache(const std::string& cachePath, const CompiledShader& shader) {
    
    std::ofstream file(cachePath, std::ios::binary);
    if (!file.is_open()) return;

    file.write(reinterpret_cast<const char*>(&shader.sourceHash), sizeof(uint64_t));

    file.write(reinterpret_cast<const char*>(&shader.num_samplers), sizeof(Uint32));
    file.write(reinterpret_cast<const char*>(&shader.num_storage_textures), sizeof(Uint32));
    file.write(reinterpret_cast<const char*>(&shader.num_storage_buffers), sizeof(Uint32));
    file.write(reinterpret_cast<const char*>(&shader.num_uniform_buffers), sizeof(Uint32));

    size_t size = shader.bytecode.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(shader.bytecode.data()), size);

    file.close();
}

bool ShaderManager::LoadFromDiskCache(const std::string& cachePath, CompiledShader& outShader) {
    
    if (!std::filesystem::exists(cachePath)) return false;

    std::ifstream file(cachePath, std::ios::binary);
    if (!file.is_open()) return false;

    file.read(reinterpret_cast<char*>(&outShader.sourceHash), sizeof(uint64_t));

    file.read(reinterpret_cast<char*>(&outShader.num_samplers), sizeof(Uint32));
    file.read(reinterpret_cast<char*>(&outShader.num_storage_textures), sizeof(Uint32));
    file.read(reinterpret_cast<char*>(&outShader.num_storage_buffers), sizeof(Uint32));
    file.read(reinterpret_cast<char*>(&outShader.num_uniform_buffers), sizeof(Uint32));

    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
    outShader.bytecode.resize(size);
    file.read(reinterpret_cast<char*>(outShader.bytecode.data()), size);

    file.close();
    return true;
}

std::vector<uint8_t> ShaderManager::ReadFile(const std::string& filepath) {
    size_t fileSize = 0;
    void* data = SDL_LoadFile(filepath.c_str(), &fileSize);

    if (!data) {
        throw std::runtime_error("Failed to load file: " + filepath);
    }

    std::vector<uint8_t> buffer(static_cast<uint8_t*>(data),
        static_cast<uint8_t*>(data) + fileSize);

    SDL_free(data);
    return buffer;
}

uint64_t ShaderManager::HashFile(const std::string& filepath) {
    auto data = ReadFile(filepath);
    std::hash<std::string> hasher;
    return hasher(std::string(data.begin(), data.end()));
}

SDL_GPUShaderStage ShaderManager::GetTargetStage(const std::string& shaderFileName) {

    if (SDL_strstr(shaderFileName.c_str(), ".vert")) {
        return SDL_GPU_SHADERSTAGE_VERTEX;
    } else if (SDL_strstr(shaderFileName.c_str(), ".frag")) {
        return SDL_GPU_SHADERSTAGE_FRAGMENT;
    } else {
        throw std::runtime_error("Unrecognized Shader Stage");
    }
}

SDL_ShaderCross_ShaderStage ShaderManager::ConvertStage(SDL_GPUShaderStage stage) {
    switch (stage) {
        case SDL_GPU_SHADERSTAGE_VERTEX:
            return SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        case SDL_GPU_SHADERSTAGE_FRAGMENT:
            return SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        default:
            throw std::runtime_error("Unsupported shader stage");
    }
}