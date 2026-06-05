#pragma once
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/Enums/PipelineEnums.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class Window;
class Viewport;
struct ImGuiIO;
struct FrameData;
struct SDL_GPUDevice;

class ViewportRenderer {
public:
	ViewportRenderer(SDL_GPUDevice* device, Viewport* viewport);
	bool Initialize();
	void Render(FrameData& frame);

	void Shutdown();
	bool IsActive() { return active; }
	bool resized = false;
	
private:
	bool InitializeBuffers();
	bool InitializeSamplers();

	SDL_GPUTextureFormat GetDepthStencilFormat();

	Viewport* viewport;

	ImGuiIO* io = nullptr;

	SDL_GPUDevice* device = nullptr;
	SDL_GPUSampler* defaultSampler = nullptr;

	SDL_GPUSampleCount sampleCount = SDL_GPU_SAMPLECOUNT_8;

	static std::unique_ptr<ShaderManager> shaderManager;

	bool msaaEnabled = true;

	bool show_demo_window = true;
	bool show_another_window = false;

	bool active = true;
};