#pragma once

#include "Core/MatrixAPI.hpp"
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <memory>

// currently not sure if asset loader needs the matrix api call? better safe than sorry but need to reevaluate and test

struct Mesh;
struct Texture;
struct Appstate;

struct SDL_GPUDevice;

class MATRIX_API AssetLoader {
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

	Texture* CreateTexture(const std::string& textureFilePath);
	Mesh* CreateMesh(const std::string& meshFilePath);
	Mesh* CreateMesh(const std::string& meshFilePath, const std::string& textureFilePath);
private:
	AssetLoader(Appstate& appstate);
    void ShutdownInternal();

	void ProcessNode(aiNode* node, const aiScene* scene, Mesh* newMesh, const aiMatrix4x4& parentTransform);
	Assimp::Importer importer;

    Appstate& appstate;

	std::vector<Mesh*> meshes;
	std::vector<Texture*> textures;

    bool isShutdown = false;

    static inline std::unique_ptr<AssetLoader> instance = nullptr;
};