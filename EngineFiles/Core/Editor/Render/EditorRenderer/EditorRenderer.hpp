#pragma once

class Viewport;
class WorldRenderer;
class Entity;
class MeshComponent;
class SpriteComponent;

struct ImGuiIO;
struct Appstate;

class EditorRenderer {
public:
	EditorRenderer(Appstate& appstate, Viewport& viewport, WorldRenderer& worldRenderer);

	void Render();
    void Shutdown();
	void Tick(float deltaTime);

    bool resized = false;
private:
	ImGuiIO* io = nullptr;

	Appstate& appstate;
	Viewport& viewport;
};