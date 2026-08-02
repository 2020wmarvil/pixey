#include "Window.h"

namespace Pixey
{
	Window::Window(const char* title, int width, int height)
	{
		handle = SDL_CreateWindow(title, width, height, SDL_WINDOW_VULKAN);
	}

	Window::~Window()
	{
		SDL_DestroyWindow(handle);
	}

	SDL_Window* Window::GetHandle() const
	{
		return handle;
	}

	void Window::HandleEvent(const SDL_Event& event)
	{
		if (event.type == SDL_EVENT_WINDOW_MINIMIZED)
		{
			bMinimized = true;
		}
		else if (event.type == SDL_EVENT_WINDOW_RESTORED)
		{
			bMinimized = false;
		}
	}

	bool Window::IsMinimized() const
	{
		return bMinimized;
	}
}
