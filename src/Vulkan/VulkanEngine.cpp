#include "VulkanEngine.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <chrono>
#include <thread>

#include "VulkanInitializers.h"
#include "VulkanTypes.h"

constexpr bool bUseValidationLayers = false;

VulkanEngine* loadedEngine = nullptr;

VulkanEngine& VulkanEngine::Get()
{
	return *loadedEngine;
}

void VulkanEngine::Initialize()
{
	// Only one engine initialization is allowed with the application.
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	// We initialize SDL and create a window with it.
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN;

	window = SDL_CreateWindow("Pixey", windowExtent.width, windowExtent.height, windowFlags);

	bInitialized = true;
}

void VulkanEngine::Shutdown()
{
	if (bInitialized)
	{
		SDL_DestroyWindow(window);
	}

	loadedEngine = nullptr;
}

void VulkanEngine::Run()
{
	SDL_Event e;
	bool bQuit = false;

	while (!bQuit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				bQuit = true;
			}
			else if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
			{
				bStopRendering = true;
			}
			else if (e.type == SDL_EVENT_WINDOW_RESTORED)
			{
				bStopRendering = false;
			}
		}

		// do not draw if we are minimized
		if (bStopRendering)
		{
			// throttle the speed to avoid the endless spinning
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		Draw();
	}
}

void VulkanEngine::Draw() {}
