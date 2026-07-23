#pragma once

#include "Core/MatrixAPI.hpp"
#include "Core/GameObject/GameObject.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_mouse.h>
#include "Controller.reflected.hpp"

using KeyBindingList = std::unordered_map<SDL_Scancode, std::function<void()>>;
using MouseButtonBindingList = std::unordered_map<uint8_t, std::function<void()>>;

CLASS()
class MATRIX_API Controller : public GameObject {
	REFLECTION()
public:
	Controller();
	~Controller() = default;

	FUNCTION()
	void Start() override;
	
	FUNCTION()
	void Tick(uint64_t deltaTime) override;
	
	FUNCTION()
	void DestroyGameObject() override;

	FUNCTION()
	bool IsKeyPressed(SDL_Scancode key);
	
	FUNCTION()
	bool IsKeyJustPressed(SDL_Scancode key);
	
	FUNCTION()
	bool IsKeyReleased(SDL_Scancode key);
	
	FUNCTION()
	void BindKey(SDL_Scancode key, 
		std::function<void()> onPress, 
		std::function<void()> onRelease = nullptr);

	FUNCTION()
	bool IsMouseButtonPressed(uint8_t button);
	
	FUNCTION()
	bool IsMouseButtonJustPressed(uint8_t button);
	
	FUNCTION()
	bool IsMouseButtonReleased(uint8_t button);
	
	FUNCTION()
	void BindMouseButton(uint8_t button, 
		std::function<void()> onPress, 
		std::function<void()> onRelease = nullptr);

	FUNCTION()
	void BindMouseMove(std::function<void(float relX, float relY)> callback);
	
	FUNCTION()
	float GetMouseX() const { return mouseX; }
	
	FUNCTION()
	float GetMouseY() const { return mouseY; }

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

	float mouseX = -1;
	float mouseY = -1;

	std::function<void(float relX, float relY)> mouseMoveCallback;

	bool keyStateChanged = false;
};