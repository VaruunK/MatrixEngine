#include "AssetLoader.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <Engine.hpp>
#include <SDL3/SDL_gpu.h>
#include <SDL3_image/SDL_image.h>

AssetLoader::AssetLoader() {
}

Texture* AssetLoader::CreateTexture(const std::string& textureFilePath) {
    SDL_Surface* imageData = IMG_Load(textureFilePath.c_str());

    if (!imageData) {
        SDL_Log("Failed to load image data: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Surface* converted = SDL_ConvertSurface(imageData, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(imageData);
    imageData = converted;

    SDL_GPUTextureCreateInfo textureCreateInfo{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = static_cast<Uint32>(imageData->w),
        .height = static_cast<Uint32>(imageData->h),
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &textureCreateInfo);
    if (!texture) {
        SDL_Log("Failed to create Texture");
        return nullptr;
    }
    SDL_SetGPUTextureName(device, texture, textureFilePath.c_str());

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = imageData->w * imageData->h * 4;

    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
    if (!transferBuffer) {
        SDL_Log("Failed to create transfer buffer");
        return nullptr;
    }

    void* mapped = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
    memcpy(mapped, imageData->pixels, transferInfo.size);
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);

    SDL_GPUTextureTransferInfo src{};
    src.transfer_buffer = transferBuffer;
    src.offset = 0;
    src.pixels_per_row = imageData->w;
    src.rows_per_layer = imageData->h;

    SDL_GPUTextureRegion dst{};
    dst.texture = texture;
    dst.w = imageData->w;
    dst.h = imageData->h;
    dst.d = 1;

    SDL_UploadToGPUTexture(copyPass, &src, &dst, false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmd);

    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    Texture* newTexture = new Texture;
    newTexture->texture = texture;
    newTexture->data = imageData;
    newTexture->texturePath = textureFilePath;

    return newTexture;
}

void AssetLoader::ProcessNode(aiNode* node, const aiScene* scene, Mesh* newMesh, const aiMatrix4x4& parentTransform) {
    aiMatrix4x4 globalTransform = parentTransform * node->mTransformation;

    for (size_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        uint32_t vertexOffset = static_cast<uint32_t>(newMesh->vertices.size());

        for (size_t j = 0; j < mesh->mNumVertices; ++j) {
            // transform vertex position by this node's global transform
            aiVector3D pos = globalTransform * mesh->mVertices[j];

            newMesh->vertices.push_back({
                .position = glm::vec3{pos.x, pos.y, pos.z},
                .uv = mesh->mTextureCoords[0]
                    ? glm::vec2{mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y}
                    : glm::vec2{0.0f, 0.0f}
                });
        }

        for (size_t j = 0; j < mesh->mNumFaces; ++j) {
            const aiFace& face = mesh->mFaces[j];
            for (size_t k = 0; k < face.mNumIndices; ++k) {
                newMesh->indices.push_back(face.mIndices[k] + vertexOffset);
            }
        }
    }

    // recurse into children
    for (size_t i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(node->mChildren[i], scene, newMesh, globalTransform);
    }
}

Mesh* AssetLoader::CreateMesh(const std::string& meshFilePath) {
    int flags = aiProcess_Triangulate;
    if (meshFilePath.ends_with(".fbx") || meshFilePath.ends_with(".FBX")) {
        flags |= aiProcess_FlipUVs;
    }
    const aiScene* scene = importer.ReadFile(meshFilePath.c_str(), flags);
    if (!scene) {
        SDL_Log("Failed to load model: %s", importer.GetErrorString());
        return nullptr;
    }

    Mesh* newMesh = new Mesh;
    aiMatrix4x4 identity;
    ProcessNode(scene->mRootNode, scene, newMesh, identity);
    newMesh->texture = CreateTexture("Content/DefaultTexture.png");
    return newMesh;
}

Mesh* AssetLoader::CreateMesh(const std::string& meshFilePath, const std::string& textureFilePath) {
    Mesh *mesh = CreateMesh(meshFilePath);
    if (!mesh) {
        return nullptr;
    }
    Texture* texture = CreateTexture(textureFilePath);
    if (!texture) {
        return nullptr;
    }
    mesh->texture = texture;
    return mesh;
}
