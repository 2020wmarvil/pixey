#pragma once

#include "Vulkan/VulkanTypes.h"

namespace Pixey
{
	class Window;

	struct DeletionQueue
	{
		void PushFunction(std::function<void()>&& function)
		{
			deletors.push_back(function);
		}

		void Flush()
		{
			// reverse iterate the deletion queue to execute all the functions
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
			{
				(*it)();
			}

			deletors.clear();
		}

		std::deque<std::function<void()>> deletors;
	};

	struct FrameData
	{
		VkCommandPool commandPool;
		VkCommandBuffer mainCommandBuffer;
		VkSemaphore swapchainSemaphore;
		VkFence renderFence;

		DeletionQueue deletionQueue;
	};

	// TODO: 3 frames + mailbox
	constexpr unsigned int FRAME_OVERLAP = 2;

	class VulkanRenderer
	{
	public:
		static VulkanRenderer& Get();

		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(const VulkanRenderer&) = delete;

		void RegisterWindow(Window& inWindow);
		void Shutdown();
		void Draw();

	private:
		VulkanRenderer() = default;

		// Vulkan initialization
		void InitVulkan();
		void InitSwapchain();
		void InitCommands();
		void InitSyncStructures();

		// Swapchain
		void CreateSwapchain(uint32_t width, uint32_t height);
		void DestroySwapchain();

		FrameData& GetCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; };

		void DrawBackground(VkCommandBuffer commandBuffer);

	private:
		Window* window = nullptr;
		VkExtent2D windowExtent = {};

		bool bInitialized = false;
		int frameNumber = 0;

		// Vulkan state
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice chosenGPU;
		VkDevice device;
		VkSurfaceKHR surface;
		VkQueue graphicsQueue;
		uint32_t graphicsQueueFamily;

		// Swapchain
		VkSwapchainKHR swapchain;
		VkFormat swapchainImageFormat;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		VkExtent2D swapchainExtent;

		// One per swapchain image, indexed by acquired image index rather than frame-in-flight.
		std::vector<VkSemaphore> renderSemaphores;

		// Frame data
		FrameData frames[FRAME_OVERLAP];

		VmaAllocator allocator;
		DeletionQueue deletionQueue;

		// Draw resources
		AllocatedImage drawImage;
		VkExtent2D drawExtent;
	};
}
