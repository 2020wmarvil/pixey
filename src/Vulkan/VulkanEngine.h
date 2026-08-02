#pragma once

#include "Vulkan/VulkanTypes.h"

namespace Pixey
{
	class Window;

	class VulkanEngine
	{
	public:
		static VulkanEngine& Get();

		VulkanEngine(const VulkanEngine&) = delete;
		VulkanEngine& operator=(const VulkanEngine&) = delete;

		void Initialize(Window* inWindow);
		void Shutdown();
		void Draw();

	private:
		VulkanEngine() = default;

	private:
		// Non-owning -- caller must keep the window alive for as long as the
		// engine stays initialized.
		Window* window = nullptr;
		VkExtent2D windowExtent = {};

		bool bInitialized = false;
		int frameNumber = 0;
	};
}
