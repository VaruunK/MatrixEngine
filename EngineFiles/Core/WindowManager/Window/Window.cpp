#include "Window.hpp"
#include "Engine.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

Window::Window(SDL_GPUDevice* device, const std::string& name, const std::string& iconFilePath, int initWinSizeX, int initWinSizeY) {
    
    this->device = device;
    // SDL_WINDOW_FULLSCREEN
    // SDL_WINDOW_BORDERLESS
    window = SDL_CreateWindow(name.c_str(), initWinSizeX, initWinSizeY, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        // throw runtime error
    }
    
    SDL_Surface* icon = IMG_Load(iconFilePath.c_str());

    if (!icon) {
        SDL_Log("couldn't load icon: %s", SDL_GetError());
    }
    else {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }

    width = initWinSizeX;
    height = initWinSizeY;

    if (!SDL_ClaimWindowForGPUDevice(this->device, window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        // throw runtime error
    }
}

bool Window::WaitAndAquireGPUSwapchainTexture(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture** swapChainTexture, 
    uint32_t* swapchain_texture_width, uint32_t* swapchain_texture_height) {

    return SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, swapChainTexture, swapchain_texture_width, swapchain_texture_height);
}

SDL_GPUTextureFormat Window::GetGPUSwapchainTextureFormat() {
    return SDL_GetGPUSwapchainTextureFormat(device, window);
}

bool Window::InitImGUI() {
    return ImGui_ImplSDL3_InitForSDLGPU(window);
}

bool Window::GetWindowSize(int *windowWidth, int *windowHeight) {
    if (!SDL_GetWindowSize(window, windowWidth, windowHeight)) {
        SDL_Log("Failed to get Window size!");
        return false;
    }
    width = *windowWidth;
    height = *windowHeight;
    return true;
}

int Window::GetWindowWidth() {
    GetWindowSize(&width, &height);
    return width;
}

int Window::GetWindowHeight() {
    GetWindowSize(&width, &height);
    return height;
}

void Window::SetWindowType() {
    
}