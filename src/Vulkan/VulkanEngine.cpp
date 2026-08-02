#include "VulkanEngine.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "VulkanInitializers.h"
#include "VulkanTypes.h"
#include "Window.h"

#include <cassert>

namespace Pixey
{
	constexpr bool bUseValidationLayers = false;

	VulkanEngine& VulkanEngine::Get()
	{
		static VulkanEngine instance;
		return instance;
	}

	void VulkanEngine::Initialize(Window* inWindow)
	{
		assert(!bInitialized);

		window = inWindow;

		int width = 0;
		int height = 0;
		SDL_GetWindowSizeInPixels(window->GetHandle(), &width, &height);
		windowExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		bInitialized = true;
	}

	void VulkanEngine::Shutdown()
	{
		bInitialized = false;
	}

	void VulkanEngine::Draw() {}
}
