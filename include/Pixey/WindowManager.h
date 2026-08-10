#pragma once

#include "Pixey/PixeyExport.h"
#include "Pixey/Window.h"

#include <SDL3/SDL.h>

#include <memory>
#include <vector>

namespace Pixey
{
	class WindowManager
	{
	public:
		PIXEY_API static WindowManager& Get();

		WindowManager(const WindowManager&) = delete;
		WindowManager& operator=(const WindowManager&) = delete;

		PIXEY_API void Initialize();
		PIXEY_API void Shutdown();

		PIXEY_API Window& OpenWindow(const char* title, int width, int height);

		PIXEY_API void HandleEvent(const SDL_Event& event);
		PIXEY_API bool AreAllWindowsMinimized() const;

	private:
		WindowManager() = default;

	private:
		std::vector<std::unique_ptr<Window>> windows;
		bool bInitialized = false;
	};
}
