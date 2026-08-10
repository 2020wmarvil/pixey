#pragma once

#include "Pixey/PixeyExport.h"
#include "Pixey/Types.h"

#include <SDL3/SDL.h>

namespace Pixey
{
	class Input
	{
	public:
		PIXEY_API static Input& Get();

		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

		PIXEY_API void Update();

		PIXEY_API bool IsKeyDown(SDL_Scancode scancode) const;
		PIXEY_API bool IsKeyPressed(SDL_Scancode scancode) const;
		PIXEY_API bool IsKeyReleased(SDL_Scancode scancode) const;

		PIXEY_API Vec2 GetMousePosition() const;
		PIXEY_API Vec2 GetMouseDelta() const;

		PIXEY_API bool IsMouseButtonDown(SDL_MouseButtonFlags buttonMask) const;
		PIXEY_API bool IsMouseButtonPressed(SDL_MouseButtonFlags buttonMask) const;
		PIXEY_API bool IsMouseButtonReleased(SDL_MouseButtonFlags buttonMask) const;

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
