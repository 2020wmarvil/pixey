#include "Input.h"

namespace Pixey
{
	void Input::Update()
	{
		for (int i = 0; i < NumScancodes; ++i)
		{
			bPreviousKeys[i] = bCurrentKeys[i];
		}

		const bool* keyboardState = SDL_GetKeyboardState(nullptr);
		for (int i = 0; i < NumScancodes; ++i)
		{
			bCurrentKeys[i] = keyboardState[i];
		}

		previousMousePos = currentMousePos;
		previousMouseButtons = currentMouseButtons;

		currentMouseButtons = SDL_GetMouseState(&currentMousePos.x, &currentMousePos.y);
	}

	bool Input::IsKeyDown(SDL_Scancode scancode) const
	{
		return bCurrentKeys[scancode];
	}

	bool Input::IsKeyPressed(SDL_Scancode scancode) const
	{
		return bCurrentKeys[scancode] && !bPreviousKeys[scancode];
	}

	bool Input::IsKeyReleased(SDL_Scancode scancode) const
	{
		return !bCurrentKeys[scancode] && bPreviousKeys[scancode];
	}

	Vec2 Input::GetMousePosition() const
	{
		return currentMousePos;
	}

	Vec2 Input::GetMouseDelta() const
	{
		return currentMousePos - previousMousePos;
	}

	bool Input::IsMouseButtonDown(SDL_MouseButtonFlags buttonMask) const
	{
		return (currentMouseButtons & buttonMask) != 0;
	}

	bool Input::IsMouseButtonPressed(SDL_MouseButtonFlags buttonMask) const
	{
		return (currentMouseButtons & buttonMask) != 0 && (previousMouseButtons & buttonMask) == 0;
	}

	bool Input::IsMouseButtonReleased(SDL_MouseButtonFlags buttonMask) const
	{
		return (currentMouseButtons & buttonMask) == 0 && (previousMouseButtons & buttonMask) != 0;
	}
}
