#pragma once

#include "Pixey/Types.h"

#include <SDL3/SDL.h>

namespace Pixey
{
	class Input
	{
	public:
		static Input& Get();

		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

		void Update();

		bool IsKeyDown(SDL_Scancode scancode) const;
		bool IsKeyPressed(SDL_Scancode scancode) const;
		bool IsKeyReleased(SDL_Scancode scancode) const;

		Vec2 GetMousePosition() const;
		Vec2 GetMouseDelta() const;

		bool IsMouseButtonDown(SDL_MouseButtonFlags buttonMask) const;
		bool IsMouseButtonPressed(SDL_MouseButtonFlags buttonMask) const;
		bool IsMouseButtonReleased(SDL_MouseButtonFlags buttonMask) const;

	private:
		Input() = default;

	private:
		static constexpr int NumScancodes = SDL_SCANCODE_COUNT;

		bool bCurrentKeys[NumScancodes] = {};
		bool bPreviousKeys[NumScancodes] = {};

		Vec2 currentMousePos = {};
		Vec2 previousMousePos = {};

		SDL_MouseButtonFlags currentMouseButtons = 0;
		SDL_MouseButtonFlags previousMouseButtons = 0;
	};
}
