#pragma once

#include "Pixey/Window.h"

#include <SDL3/SDL.h>

#include <memory>
#include <vector>

namespace Pixey
{
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
		bool AreAllWindowsMinimized() const;

	private:
		WindowManager() = default;

	private:
		std::vector<std::unique_ptr<Window>> windows;
		bool bInitialized = false;
	};
}
