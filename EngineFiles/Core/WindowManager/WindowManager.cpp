#include "WindowManager.hpp"
#include "Window/Window.hpp"
#include <imgui.h>

WindowManager::WindowManager(SDL_GPUDevice* device) {
	//ImGuiIO &io = ImGui::GetIO();
	// io.DisplaySize.x, io.DisplaySize.y
	mainWindow = new Window(device, "Matrix Engine", "Engine.png", 1080, 720);
	windows.push_back(mainWindow);
}

WindowManager::~WindowManager() {

}

Window* WindowManager::CreateWindow()
{
	return nullptr;
}
