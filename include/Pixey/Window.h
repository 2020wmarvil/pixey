#pragma once

#include "Pixey/PixeyExport.h"

#include <SDL3/SDL.h>

namespace Pixey
{
	// May only be constructed by WindowManager, so that a window's lifetime always stays tracked.
	class Window
	{
	public:
		PIXEY_API ~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		PIXEY_API SDL_Window* GetHandle() const;

		PIXEY_API void HandleEvent(const SDL_Event& event);
		PIXEY_API bool IsMinimized() const;

	private:
		Window(const char* title, int width, int height);
		friend class WindowManager;

	private:
		SDL_Window* handle = nullptr;
		bool bMinimized = false;
	};
}
