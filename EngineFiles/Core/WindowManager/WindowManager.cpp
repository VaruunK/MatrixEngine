#include "WindowManager.hpp"
#include "Window/Window.hpp"

WindowManager::WindowManager(SDL_GPUDevice* device) {
	mainWindow = std::make_unique<Window>(device, "Matrix Engine", "Engine.png", 1080, 720);
	windows.push_back(mainWindow.get());
}

WindowManager::~WindowManager() {

}

Window* WindowManager::CreateWindow()
{
	return nullptr;
}
