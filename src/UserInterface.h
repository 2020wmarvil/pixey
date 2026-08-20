#pragma once

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

namespace Pixey
{
	class Window;

	class UserInterface
	{
	public:
		static UserInterface& Get();

		UserInterface(const UserInterface&) = delete;
		UserInterface& operator=(const UserInterface&) = delete;

		void Init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamily, VkQueue queue, VkFormat colorFormat, Window& window);
		void Shutdown();

		void ProcessEvent(const SDL_Event& event);
		void NewFrame();
		void Render(VkCommandBuffer commandBuffer, VkImageView targetImageView, VkExtent2D extent);

	private:
		UserInterface() = default;

	private:
		bool bInitialized = false;
	};
}
