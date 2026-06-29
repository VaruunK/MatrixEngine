#include "AssetLoader.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include "Core/Structs/AssetStructs.hpp"
#include "Core/Structs/Appstate.hpp"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3_image/SDL_image.h>

AssetLoader::AssetLoader(Appstate& appstate) : appstate(appstate) {

}

void AssetLoader::ShutdownInternal() {
    for (auto* texture : textures) {
        if (texture) {
            if (texture->texture) {
                SDL_ReleaseGPUTexture(appstate.device, texture->texture);
                texture->texture = nullptr;
            }
            if (texture->data) {
                SDL_DestroySurface(texture->data);
                texture->data = nullptr;
            }
            delete texture;
            texture = nullptr;
        }
    }
    for (auto* mesh : meshes) {
        if (mesh) {
            mesh->vertices.clear();
            mesh->indices.clear();
            delete mesh;
            mesh = nullptr;
        }
    }
    textures.clear();
    meshes.clear();
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

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_R_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_G_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_B_FLOAT, 0.0f);
    SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_A_FLOAT, 1.0f);

    SDL_GPUTextureCreateInfo textureCreateInfo{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = static_cast<Uint32>(imageData->w),
        .height = static_cast<Uint32>(imageData->h),
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .props = props
    };

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(appstate.device, &textureCreateInfo);
    SDL_DestroyProperties(props);

    if (!texture) {
        SDL_Log("Failed to create Texture");
        return nullptr;
    }

    SDL_SetGPUTextureName(appstate.device, texture, textureFilePath.c_str());

    const uint32_t PITCH_ALIGNMENT = 256;
    uint32_t rawPitch = static_cast<uint32_t>(imageData->w) * 4;
    uint32_t alignedPitch = (rawPitch + PITCH_ALIGNMENT - 1) & ~(PITCH_ALIGNMENT - 1);
    uint32_t transferSize = alignedPitch * static_cast<uint32_t>(imageData->h);

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = transferSize;

    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(appstate.device, &transferInfo);
    if (!transferBuffer) {
        SDL_Log("Failed to create transfer buffer");
        return nullptr;
    }

    uint8_t* mapped = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(appstate.device, transferBuffer, false));
    const uint8_t* pixels = static_cast<const uint8_t*>(imageData->pixels);

    for (int row = 0; row < imageData->h; row++) {
        memcpy(mapped + row * alignedPitch,
            pixels + row * rawPitch,
            rawPitch);
    }

    SDL_UnmapGPUTransferBuffer(appstate.device, transferBuffer);

    SDL_GPUCommandBuffer* uploadCmd = SDL_AcquireGPUCommandBuffer(appstate.device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmd);

    SDL_GPUTextureTransferInfo src{};
    src.transfer_buffer = transferBuffer;
    src.offset = 0;
    src.pixels_per_row = alignedPitch / 4;
    src.rows_per_layer = static_cast<uint32_t>(imageData->h);

    SDL_GPUTextureRegion dst{};
    dst.texture = texture;
    dst.w = static_cast<uint32_t>(imageData->w);
    dst.h = static_cast<uint32_t>(imageData->h);
    dst.d = 1;

    SDL_UploadToGPUTexture(copyPass, &src, &dst, false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmd);
    SDL_ReleaseGPUTransferBuffer(appstate.device, transferBuffer);

    Texture* newTexture = new Texture;
    newTexture->texture = texture;
    newTexture->data = imageData;
    newTexture->texturePath = textureFilePath;

    if (newTexture) {
        textures.push_back(newTexture);
    }

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

    if (newMesh) {
        meshes.push_back(newMesh);
    }

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
