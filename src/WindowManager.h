#pragma once

#include "Window.h"

#include <SDL3/SDL.h>

#include <memory>
#include <vector>

namespace Pixey
{
	// Owns the SDL video subsystem (SDL_Init/SDL_Quit) and every Window it opens.
	class WindowManager
	{
	public:
		static WindowManager& Get();

		WindowManager(const WindowManager&) = delete;
		WindowManager& operator=(const WindowManager&) = delete;

		void Initialize();
		void Shutdown();

		Window& OpenWindow(const char* title, int width, int height);

		void HandleEvent(const SDL_Event& event);

	private:
		WindowManager() = default;

	private:
		std::vector<std::unique_ptr<Window>> windows;
		bool bInitialized = false;
	};
}
