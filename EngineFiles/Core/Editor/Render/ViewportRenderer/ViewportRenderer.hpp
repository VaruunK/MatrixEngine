#pragma once
#include "Core/ShaderManager/ShaderManager.hpp"
#include "Core/Enums/PipelineEnums.hpp"
#include "Core/Structs/RenderStructs.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class Viewport;

struct ImGuiIO;
struct FrameData;
struct Appstate;

class ViewportRenderer {
public:
	ViewportRenderer(Appstate& appstate, Viewport* viewport);
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

	Appstate& appstate;

	SDL_GPUSampler* defaultSampler = nullptr;

	SDL_GPUTexture* selectProxyTexture = nullptr;

	static std::unique_ptr<ShaderManager> shaderManager;

	bool msaaEnabled = true;

	bool show_demo_window = true;
	bool show_another_window = false;

	bool active = true;
	bool showFPS = false;
	int speed = 1;

};