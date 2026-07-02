#pragma once

#ifdef MATRIX_EDITOR

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <memory>
#include <filesystem>

struct Mesh;
struct Texture;
struct Material;
struct Appstate;

struct SDL_GPUDevice;
struct SDL_GPUTexture;

class AssetLoader {
public:
    static AssetLoader& Get() {
        return *instance;
    }
    
    ~AssetLoader() {
        if (!isShutdown) {
            Shutdown();
        }
    }

    static void Init(Appstate& appstate) {
        instance.reset(new AssetLoader(appstate));
    }

    static void Shutdown() {
        if (instance) {
            instance->ShutdownInternal();
            instance.reset();
        }
    }

    Texture* ImportTexture(const std::filesystem::path& textureFilePath);
    
    Mesh* ImportMesh(const std::filesystem::path& meshFilePath);
    void WriteMesh(const std::filesystem::path& meshFilePath, Mesh* mesh);

    Mesh* ReadMesh(const std::filesystem::path& meshFilePath);

private:
    struct MeshFileHeader {
        uint32_t magic = 0x4D455348;
        uint32_t version = 1;
        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;
    };

	AssetLoader(Appstate& appstate);

    void WriteTexture(const std::filesystem::path& meshFilePath, Material* texture);

    void ShutdownInternal();

	void ProcessNode(aiNode* node, const aiScene* scene, Mesh* newMesh, const aiMatrix4x4& parentTransform);
	Assimp::Importer importer;

    Appstate& appstate;

    bool isShutdown = false;

    // Material* defaultMaterial = nullptr;

    static inline std::unique_ptr<AssetLoader> instance = nullptr;
};

#endif