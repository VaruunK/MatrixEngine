#pragma once

#ifdef MATRIX_EDITOR

class WorldRenderer;
class Entity;
class MeshComponent;
class SpriteComponent;

struct EditorInfo;
struct ImGuiIO;
struct Appstate;
struct SDL_GPUTexture;

class EditorRenderer {
public:
	EditorRenderer(Appstate& appstate, EditorInfo& info, WorldRenderer& worldRenderer);

	void Render();
    void Shutdown();
	void Tick(float deltaTime);

    bool resized = false;
private:
	void RenderMenuBar();
	
	ImGuiIO* io = nullptr;

	EditorInfo& info;

	SDL_GPUTexture* swapchainTexture = nullptr;

	bool contentBrowserOpen = true;
	bool contentBrowserViewOption = false;
	bool contentBrowserLocked = true;

	Appstate& appstate;
};

#endif