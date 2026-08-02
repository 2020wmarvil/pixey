#pragma once

#include "Vulkan/VulkanTypes.h"

class VulkanEngine
{
public:
	static VulkanEngine& Get();

	void Initialize();
	void Shutdown();
	void Run();
	void Draw();

	struct SDL_Window* window = nullptr;
	VkExtent2D windowExtent = { 1700, 900 };

	bool bInitialized = false;
	bool bStopRendering = false;
	int frameNumber = 0;
};
