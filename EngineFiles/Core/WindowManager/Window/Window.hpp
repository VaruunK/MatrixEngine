#pragma once

#include <glm/glm.hpp>
#include <string>

struct SDL_Window;
struct SDL_GPUCommandBuffer;
struct SDL_GPUTexture;
struct SDL_GPUDevice;
enum SDL_GPUTextureFormat;

class Window {
public:
	Window(SDL_GPUDevice* device, const std::string& name, const std::string& iconFilePath, int initWinSizeX, int initWinSizeY);
	~Window();

	bool WaitAndAquireGPUGwapchainTexture(SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture** swapChainTexture, 
		uint32_t* swapchain_texture_width, uint32_t* swapchain_texture_height);

	SDL_GPUTextureFormat GetGPUSwapchainTextureFormat();

	bool GetWindowSize(int *windowWidth, int *windowHeight);
	int GetWindowWidth();
	int GetWindowHeight();
	void SetWindowType();
private:
	SDL_GPUDevice* device;
	SDL_Window* window;
	int width;
	int height;
};