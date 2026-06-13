#pragma once

struct SDL_GPUCommandBuffer;
struct SDL_GPUTexture;
struct View;

struct FrameData {
    SDL_GPUCommandBuffer* commandBuffer = nullptr;
    SDL_GPUTexture* swapchainTexture = nullptr;
    SDL_GPUTexture* viewportTexture = nullptr;
    const View* view = nullptr;
};