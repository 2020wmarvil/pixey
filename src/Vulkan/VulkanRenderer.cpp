#include "VulkanRenderer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "VkBootstrap.h"

#include "VulkanInitializers.h"
#include "VulkanTypes.h"
#include "VulkanImages.h"
#include "Pixey/Window.h"

#include <cassert>

namespace Pixey
{
	constexpr bool bUseValidationLayers = true;

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

		// TODO: Move these out of window registration
		InitVulkan();
		InitSwapchain();
		InitCommands();
		InitSyncStructures();

		bInitialized = true;
	}

	void VulkanRenderer::Shutdown()
	{
		if (bInitialized)
		{
			// Make sure the gpu has stopped doing its things.
			vkDeviceWaitIdle(device);

			for (int i = 0; i < FRAME_OVERLAP; i++)
			{
				vkDestroyCommandPool(device, frames[i].commandPool, nullptr);
				vkDestroyFence(device, frames[i].renderFence, nullptr);
				vkDestroySemaphore(device ,frames[i].swapchainSemaphore, nullptr);
			}

			DestroySwapchain();

			vkDestroySurfaceKHR(instance, surface, nullptr);
			vkDestroyDevice(device, nullptr);

			vkb::destroy_debug_utils_messenger(instance, debugMessenger);
			vkDestroyInstance(instance, nullptr);
		}

		bInitialized = false;
	}

	void VulkanRenderer::Draw()
	{
		const FrameData& currentFrame = GetCurrentFrame();

		// Wait until the gpu has finished rendering the last frame. Timeout of 1 second.
		VK_CHECK(vkWaitForFences(device, 1, &currentFrame.renderFence, true, 1000000000));
		VK_CHECK(vkResetFences(device, 1, &currentFrame.renderFence));

		// Request image from the swapchain.
		uint32_t swapchainImageIndex;
		VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, currentFrame.swapchainSemaphore, nullptr, &swapchainImageIndex));

		VkCommandBuffer commandBuffer = currentFrame.mainCommandBuffer;

		// Reset the command buffer to begin recording this frame.
		VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

		// Begin recording into the command buffer.
		VkCommandBufferBeginInfo commandBufferBeginInfo = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		// Transition swapchain image into writeable mode before rendering.
		VulkanUtils::TransitionImage(commandBuffer, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		VkClearColorValue clearValue;
		float flash = std::abs(std::sin(frameNumber / 120.f));
		clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

		VkImageSubresourceRange clearRange = VulkanInitializers::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		// Clear image.
		vkCmdClearColorImage(commandBuffer, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

		// Make the swapchain image into presentable mode.
		VulkanUtils::TransitionImage(commandBuffer, swapchainImages[swapchainImageIndex],VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		// Finish recording the command buffer.
		VK_CHECK(vkEndCommandBuffer(commandBuffer));

		// Prepare the submission to the queue.
		// We want to wait on the presentSemaphore, as that semaphore is signaled when the swapchain is ready.
		// We will signal the renderSemaphore, to signal that rendering has finished.
		VkCommandBufferSubmitInfo submitInfo = VulkanInitializers::CommandBufferSubmitInfo(commandBuffer);
		VkSemaphoreSubmitInfo waitInfo = VulkanInitializers::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrame.swapchainSemaphore);
		VkSemaphoreSubmitInfo signalInfo = VulkanInitializers::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, renderSemaphores[swapchainImageIndex]);

		VkSubmitInfo2 submit = VulkanInitializers::SubmitInfo(&submitInfo, &signalInfo, &waitInfo);

		// Submit command buffer to the queue and execute it.
		// renderFence will now block until the graphic commands finish execution.
		VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submit, currentFrame.renderFence));

		// Wait for renderSemaphore before presenting the drawn image to the window.
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.swapchainCount = 1;
		presentInfo.pWaitSemaphores = &renderSemaphores[swapchainImageIndex];
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &swapchainImageIndex;

		VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));

		frameNumber++;
	}

	void VulkanRenderer::InitVulkan()
	{
		vkb::InstanceBuilder builder;

		auto inst_ret = builder.set_app_name("Pixey")
			.request_validation_layers(bUseValidationLayers)
			.use_default_debug_messenger()
			.require_api_version(1, 3, 0)
			.build();

		vkb::Instance vkb_inst = inst_ret.value();

		instance = vkb_inst.instance;
		debugMessenger = vkb_inst.debug_messenger;

		SDL_Vulkan_CreateSurface(window->GetHandle(), instance, nullptr, &surface);

		// Vulkan 1.3 features
		VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features.dynamicRendering = true;
		features.synchronization2 = true;

		// Vulkan 1.2 features
		VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		features12.bufferDeviceAddress = true;
		features12.descriptorIndexing = true;

		// We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features.
		vkb::PhysicalDeviceSelector selector{ vkb_inst };
		vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 3)
			.set_required_features_13(features)
			.set_required_features_12(features12)
			.set_surface(surface)
			.select()
			.value();

		// Create the final vulkan device.
		vkb::DeviceBuilder deviceBuilder{ physicalDevice };
		vkb::Device vkbDevice = deviceBuilder.build().value();

		device = vkbDevice.device;
		chosenGPU = physicalDevice.physical_device;
		graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	}

	void VulkanRenderer::InitSwapchain()
	{
		CreateSwapchain(windowExtent.width, windowExtent.height);
	}

	void VulkanRenderer::InitCommands()
	{
		VkCommandPoolCreateInfo commandPoolInfo = VulkanInitializers::CommandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		for (int i = 0; i < FRAME_OVERLAP; i++) {

			VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frames[i].commandPool));

			// Allocate the default command buffer that we will use for rendering.
			VkCommandBufferAllocateInfo cmdAllocInfo = VulkanInitializers::CommandBufferAllocateInfo(frames[i].commandPool, 1);

			VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frames[i].mainCommandBuffer));
		}
	}

	void VulkanRenderer::InitSyncStructures()
	{
		// One fence to control when the gpu has finished rendering the frame, and 2 semaphores to synchronize rendering with swapchain.
		// We want the fence to start signaled so we can wait on it on the first frame.
		VkFenceCreateInfo fenceCreateInfo = VulkanInitializers::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
		VkSemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();

		for (int i = 0; i < FRAME_OVERLAP; i++)
		{
			VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence));

			VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].swapchainSemaphore));
		}
	}

	void VulkanRenderer::CreateSwapchain(uint32_t width, uint32_t height)
	{
		vkb::SwapchainBuilder swapchainBuilder{ chosenGPU, device, surface };

		swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			//.use_default_format_selection()
			.set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
			//use vsync present mode
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(width, height)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

		swapchainExtent = vkbSwapchain.extent;
		swapchain = vkbSwapchain.swapchain;
		swapchainImages = vkbSwapchain.get_images().value();
		swapchainImageViews = vkbSwapchain.get_image_views().value();

		VkSemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();
		renderSemaphores.resize(swapchainImages.size());
		for (int i = 0; i < renderSemaphores.size(); i++)
		{
			VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphores[i]));
		}
	}

	void VulkanRenderer::DestroySwapchain()
	{
		vkDestroySwapchainKHR(device, swapchain, nullptr);

		for (int i = 0; i < swapchainImageViews.size(); i++)
		{
			vkDestroyImageView(device, swapchainImageViews[i], nullptr);
		}

		for (int i = 0; i < renderSemaphores.size(); i++)
		{
			vkDestroySemaphore(device, renderSemaphores[i], nullptr);
		}
		renderSemaphores.clear();
	}
}
