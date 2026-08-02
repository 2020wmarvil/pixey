#include "VulkanRenderer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "Pixey/Window.h"
#include "VulkanInitializers.h"
#include "VulkanTypes.h"

#include <cassert>

namespace Pixey
{
	constexpr bool bUseValidationLayers = false;

	VulkanRenderer& VulkanRenderer::Get()
	{
		static VulkanRenderer instance;
		return instance;
	}

	void VulkanRenderer::RegisterWindow(Window& inWindow)
	{
		assert(!bInitialized);

		window = &inWindow;

		int width = 0;
		int height = 0;
		SDL_GetWindowSizeInPixels(window->GetHandle(), &width, &height);
		windowExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		bInitialized = true;
	}

	void VulkanRenderer::Shutdown()
	{
		bInitialized = false;
	}

	void VulkanRenderer::Draw() {}
}
