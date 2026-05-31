#pragma once
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/Render/PipelineEnums.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class Window;
struct ImGuiIO;

class EngineRenderer {
public:
	EngineRenderer(SDL_GPUDevice* device);
	bool Initialize();
	void Render();

	void Shutdown();

	bool resized = false;
private:
	bool InitializeBuffers();
	bool InitializePipelines(Window* window, SDL_GPUShader* vertexShader, SDL_GPUShader* fragmentShader);
	bool InitializeSamplers();
	bool CreateDepthStencil(Window* window);
	bool CreateMSAATexture(Window* window);

	SDL_GPUTextureFormat GetDepthStencilFormat();

	ImGuiIO* io = nullptr;

	SDL_GPUDevice* device = nullptr;
	SDL_GPUSampler* defaultSampler = nullptr;
	SDL_GPUTexture* depthStencilTexture = nullptr;
	SDL_GPUTexture* msaaTexture = nullptr;

	SDL_GPUSampleCount sampleCount = SDL_GPU_SAMPLECOUNT_8;

	std::unordered_map<PIPELINE_TYPE, SDL_GPUGraphicsPipeline*> pipelines;

	static std::unique_ptr<ShaderManager> shaderManager;

	bool msaaEnabled = true;

	bool show_demo_window = true;
	bool show_another_window = false;
};