#pragma once

#include "Vulkan/VulkanTypes.h"

namespace Pixey
{
	class Window;

	class VulkanRenderer
	{
	public:
		static VulkanRenderer& Get();

		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(const VulkanRenderer&) = delete;

		void Initialize(Window* inWindow);
		void Shutdown();
		void Draw();

	private:
		VulkanRenderer() = default;

	private:
		Window* window = nullptr;
		VkExtent2D windowExtent = {};

		bool bInitialized = false;
		int frameNumber = 0;
	};
}
