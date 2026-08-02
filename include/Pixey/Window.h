#pragma once

#include <SDL3/SDL.h>

namespace Pixey
{
	// May only be constructed by WindowManager, so that a window's lifetime always stays tracked.
	class Window
	{
	public:
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		SDL_Window* GetHandle() const;

		void HandleEvent(const SDL_Event& event);
		bool IsMinimized() const;

	private:
		Window(const char* title, int width, int height);
		friend class WindowManager;

	private:
		SDL_Window* handle = nullptr;
		bool bMinimized = false;
	};
}
