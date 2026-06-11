#pragma once

#include "Core/GameObject/World/World.hpp"
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/PhysicsManager/PhysicsManager.hpp"
#include "Core/Game/Game.hpp"
#include "Core/Assets/AssetLoader/AssetLoader.hpp"
#include "Core/Viewport/Viewport.hpp"
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
	SDL_GPUDevice& GetGPUDevice() { return *device; }

private:
	// need deleters for other unique pointers, probably mandatory


	struct SDLGPUDeviceDeleter {
		void operator()(SDL_GPUDevice* d) const {
			if (d) SDL_DestroyGPUDevice(d);
		}
	};

	Engine();
	
	void CreateDevice();
	void CreateWindow();

	std::atomic<bool> running;
	int MAX_FRAMES = 144;
	const int MAX_PHYSICS_FRAMES = 60;

	Viewport* viewport;
	SDL_Window* window;
	AssetLoader* assetLoader;

	Game* game;

	SDL_GPUCommandBuffer* currentCommandBuffer = nullptr;
	SDL_GPUTexture* currentSwapchainTexture = nullptr;

	std::unique_ptr<SDL_GPUDevice, SDLGPUDeviceDeleter> device;
	std::vector<std::thread> threads;
};