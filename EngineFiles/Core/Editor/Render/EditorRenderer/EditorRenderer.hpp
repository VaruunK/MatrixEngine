#pragma once

class WorldRenderer;
class Entity;
class MeshComponent;
class SpriteComponent;

struct EditorInfo;
struct ImGuiIO;
struct Appstate;

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

	bool contentBrowserOpen = true;
	bool contentBrowserViewOption = false;
	bool contentBrowserLocked = true;

	Appstate& appstate;
};