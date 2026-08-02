#include "Pixey/WindowManager.h"

#include "Vulkan/VulkanRenderer.h"

#include <SDL3/SDL.h>

#include <cassert>

namespace Pixey
{
	WindowManager& WindowManager::Get()
	{
		static WindowManager instance;
		return instance;
	}

	void WindowManager::Initialize()
	{
		assert(!bInitialized);

		SDL_Init(SDL_INIT_VIDEO);

		bInitialized = true;
	}

	void WindowManager::Shutdown()
	{
		// Windows must be destroyed before SDL_Quit()
		windows.clear();

		SDL_Quit();

		bInitialized = false;
	}

	Window& WindowManager::OpenWindow(const char* title, int width, int height)
	{
		windows.emplace_back(new Window(title, width, height));
		Window& window = *windows.back();

		VulkanRenderer::Get().RegisterWindow(window);

		return window;
	}

	void WindowManager::HandleEvent(const SDL_Event& event)
	{
		if (event.type != SDL_EVENT_WINDOW_MINIMIZED && event.type != SDL_EVENT_WINDOW_RESTORED)
		{
			return;
		}

		for (const std::unique_ptr<Window>& window : windows)
		{
			if (SDL_GetWindowID(window->GetHandle()) == event.window.windowID)
			{
				window->HandleEvent(event);
				break;
			}
		}
	}

	bool WindowManager::AreAllWindowsMinimized() const
	{
		if (windows.empty())
		{
			return false;
		}

		for (const std::unique_ptr<Window>& window : windows)
		{
			if (!window->IsMinimized())
			{
				return false;
			}
		}

		return true;
	}
}
