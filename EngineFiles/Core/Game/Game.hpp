#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/Structs/Appstate.hpp"
#include "Core/GameObject/World/World.hpp"
#include <SDL3/SDL_gpu.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

class MATRIX_API Game {
public:
	Game();
	Game(Appstate& appstate);
	~Game();

	// No copy
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;

	// No move
	Game(Game&&) = delete;
	Game& operator=(Game&&) = delete;

	void Initialize(std::string& name, std::string& iconFilePath);
	
	virtual void Start();
	virtual void Tick(float deltaTime);
	virtual void Shutdown();

	World world;
private:
	SDL_GPUDevice* CreateDevice();
	SDL_Window* CreateWindow();
	
	Appstate appstate;

	std::string iconFilePath;
	std::string name;
	std::vector<std::thread> threads;

	bool ownsAppstate = false;
	std::atomic<bool> running = false;
};