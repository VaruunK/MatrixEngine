#pragma once

#include "Core/Viewport/ViewportController/ViewportController.hpp"
#include "Core/Viewport/ViewportRenderer/ViewportRenderer.hpp"
#include "Core/Structs/FrameData.hpp"

struct View;
struct SDL_GPUDevice;

class Viewport {
public:
	Viewport(SDL_GPUDevice* device);
	~Viewport() = default;

	void Initialize();
	void Render(FrameData& frame);

	void SetCameraSpeed(int& speed);

	const View& GetCameraView() const;
private:

	ViewportController controller;
	ViewportRenderer renderer;
};