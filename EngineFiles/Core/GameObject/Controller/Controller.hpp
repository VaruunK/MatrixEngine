#pragma once

#include "Core/GameObject/GameObject.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <SDL3/SDL_scancode.h>

using BindingList = std::unordered_map<SDL_Scancode, std::function<void()>>;

class Controller : public GameObject {
public:
	Controller();
	~Controller() = default;

	void Start() override;
	void Tick(uint64_t deltaTime) override;
	void DestroyGameObject() override;

	bool IsKeyPressed(SDL_Scancode PressedKey);
	bool IsKeyReleased(SDL_Scancode PressedKey);
	void BindKey(SDL_Scancode key,
		std::function<void()> onPress,
		std::function<void()> onRelease = nullptr);

	bool StateChanged() { return stateChanged; }
	
private:
	void handleInput();
	std::vector<bool> state;
	std::vector<bool> previousState;
	int numKeys;

	BindingList onPressedBindings;
	BindingList onReleaseBindings;

	bool stateChanged = false;
};