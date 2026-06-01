#pragma once

#include "Core/GameObject/GameObject.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_mouse.h>

using KeyBindingList = std::unordered_map<SDL_Scancode, std::function<void()>>;
using MouseButtonBindingList = std::unordered_map<uint8_t, std::function<void()>>;

class Controller : public GameObject {
public:
	Controller();
	~Controller() = default;

	void Start() override;
	void Tick(uint64_t deltaTime) override;
	void DestroyGameObject() override;

	bool IsKeyPressed(SDL_Scancode key);
	bool IsKeyJustPressed(SDL_Scancode key);
	bool IsKeyReleased(SDL_Scancode key);
	void BindKey(SDL_Scancode key,
		std::function<void()> onPress,
		std::function<void()> onRelease = nullptr);

	bool IsMouseButtonPressed(uint8_t button);
	bool IsMouseButtonJustPressed(uint8_t button);
	bool IsMouseButtonReleased(uint8_t button);
	void BindMouseButton(uint8_t button, 
		std::function<void()> onPress,
		std::function<void()> onRelease = nullptr);

	void BindMouseMove(std::function<void(float relX, float relY)> callback);

	bool KeyStateChanged() { return keyStateChanged; }
	
private:
	void handleInput();
	std::vector<bool> keyState;
	std::vector<bool> previousKeyState;
	int numKeys;

	SDL_MouseButtonFlags mouseButtonState;
	SDL_MouseButtonFlags previousMouseButtonState;

	KeyBindingList onPressedKeyBindings;
	KeyBindingList onReleaseKeyBindings;

	MouseButtonBindingList onPressedMouseButtonBindings;
	MouseButtonBindingList onReleasedMouseButtonBindings;

	std::function<void(float relX, float relY)> mouseMoveCallback;

	bool keyStateChanged = false;
};