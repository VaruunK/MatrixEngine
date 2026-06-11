#include "Game.hpp"
#include "Core/GameObject/World/World.hpp"
#include <SDL3_image/SDL_image.h>


Game::Game() {
    world = new World();
}

void Game::Initialize(std::string& name, std::string& iconFilePath) {
    world->Initialize("Mainlevel", device, window);
}

void Game::Start() {
    CreateDevice();
    CreateWindow();
    running.store(true);
}

void Game::CreateDevice() {
    device = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
        false,
        nullptr
    );

    if (!device) {
        throw std::runtime_error("Failed to create GPU device");
    }
}

void Game::CreateWindow() {
    // SDL_WINDOW_FULLSCREEN
    // SDL_WINDOW_BORDERLESS
    window = SDL_CreateWindow("Game", 1080, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        // throw runtime error
    }

    SDL_Surface* icon = IMG_Load("Engine.png");

    if (!icon) {
        SDL_Log("couldn't load icon: %s", SDL_GetError());
    }
    else {
        SDL_SetWindowIcon(window, icon);
        SDL_DestroySurface(icon);
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        SDL_Log("Failed to claim window: %s", SDL_GetError());
        // throw runtime error
    }
}

//int main() {
//    Game* game = new Game();
//    game->Start();
//
//	return 0;
//}