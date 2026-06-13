#pragma once

#include "Core/GameObject/World/World.hpp"
#include "Core/Game/Game.hpp"
#include "Core/Assets/AssetLoader/AssetLoader.hpp"
#include "Core/Editor/Editor.hpp"
#include "Core/Structs/Appstate.hpp"
#include <memory>
#include <atomic>

class Engine {
public:
	static Engine& GetEngine() {
		static Engine engine;
		return engine;
	}

	int Run();
	
	Engine(Engine const&) = delete;
	void operator=(Engine const&) = delete;

	bool IsRunning() { return running.load(); };
	void SetMaxFrames(int frames) { MAX_FRAMES = frames; }

	AssetLoader& GetAssetLoader() { return *assetLoader; }
private:

	Engine();
	
	SDL_GPUDevice* CreateDevice();
	SDL_Window* CreateWindow();

	std::atomic<bool> running;
	int MAX_FRAMES = 144;
	const int MAX_PHYSICS_FRAMES = 60;

	Editor* editor;
	AssetLoader* assetLoader;

	Game* game;

	Appstate appstate;

	SDL_GPUCommandBuffer* currentCommandBuffer = nullptr;
	SDL_GPUTexture* currentSwapchainTexture = nullptr;

	std::vector<std::thread> threads;
};