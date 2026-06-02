#include "WindowManager.hpp"
#include "Window/Window.hpp"

WindowManager::WindowManager(SDL_GPUDevice* device) {
	mainWindow = new Window(device, "Matrix Engine", "Engine.png", 1080, 720);
	windows.push_back(mainWindow);
}

WindowManager::~WindowManager() {

}

Window* WindowManager::CreateWindow()
{
	return nullptr;
}
