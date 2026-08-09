#include "VulkanRenderer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "VkBootstrap.h"
#define VMA_IMPLEMENTATION
#include "Pixey/Window.h"

#include <cassert>

#include "vk_mem_alloc.h"
#include "VulkanImages.h"
#include "VulkanInitializers.h"
#include "VulkanPipelines.h"
#include "VulkanTypes.h"

#include "Pixey/Log.h"

#include "Pixey/Shaders/gradient_comp.spv.h"

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
		InitDescriptors();
		InitPipelines();

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

				frames[i].deletionQueue.Flush();
			}

			deletionQueue.Flush();

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
		FrameData& currentFrame = GetCurrentFrame();

		// Wait until the gpu has finished rendering the last frame. Timeout of 1 second.
		VK_CHECK(vkWaitForFences(device, 1, &currentFrame.renderFence, true, 1000000000));
		VK_CHECK(vkResetFences(device, 1, &currentFrame.renderFence));

		currentFrame.deletionQueue.Flush();

		// Request image from the swapchain.
		uint32_t swapchainImageIndex;
		VK_CHECK(vkAcquireNextImageKHR(device, swapchain, 1000000000, currentFrame.swapchainSemaphore, nullptr, &swapchainImageIndex));

		VkCommandBuffer commandBuffer = currentFrame.mainCommandBuffer;

		// Reset the command buffer to begin recording this frame.
		VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

		// Begin recording into the command buffer.
		VkCommandBufferBeginInfo commandBufferBeginInfo = VulkanInitializers::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

		drawExtent.width = drawImage.imageExtent.width;
		drawExtent.height = drawImage.imageExtent.height;

		// Transition image into writeable mode before rendering.
		VulkanUtils::TransitionImage(commandBuffer, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

		DrawBackground(commandBuffer);

		// Transition the draw image and the swapchain image into their correct transfer layouts.
		VulkanUtils::TransitionImage(commandBuffer, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VulkanUtils::TransitionImage(commandBuffer, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Execute a copy from the draw image into the swapchain.
		VulkanUtils::CopyImageToImage(commandBuffer, drawImage.image, swapchainImages[swapchainImageIndex], drawExtent, swapchainExtent);

		// Set swapchain image layout to Present so we can show it on the screen.
		VulkanUtils::TransitionImage(commandBuffer, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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

	void VulkanRenderer::DrawBackground(VkCommandBuffer commandBuffer)
	{
		VkClearColorValue clearValue;
		float flash = std::abs(std::sin(frameNumber / 120.f));
		clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

		VkImageSubresourceRange clearRange = VulkanInitializers::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

		// Clear image.
		vkCmdClearColorImage(commandBuffer, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
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

		// Initialize the memory allocator.
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = chosenGPU;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &allocator);

		deletionQueue.PushFunction([&]() { vmaDestroyAllocator(allocator); });
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

	void VulkanRenderer::InitDescriptors()
	{
		// Create a descriptor pool that will hold 10 sets with 1 image each.
		std::vector<DescriptorAllocator::PoolSizeRatio> sizes =
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
		};

		globalDescriptorAllocator.InitPool(device, 10, sizes);

		// Make the descriptor set layout for our compute draw.
		{
			DescriptorLayoutBuilder builder;
			builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
			drawImageDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_COMPUTE_BIT);
		}

		// Allocate a descriptor set for our draw image.
		drawImageDescriptors = globalDescriptorAllocator.Allocate(device, drawImageDescriptorLayout);

		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imgInfo.imageView = drawImage.imageView;

		VkWriteDescriptorSet drawImageWrite = {};
		drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		drawImageWrite.pNext = nullptr;

		drawImageWrite.dstBinding = 0;
		drawImageWrite.dstSet = drawImageDescriptors;
		drawImageWrite.descriptorCount = 1;
		drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		drawImageWrite.pImageInfo = &imgInfo;

		vkUpdateDescriptorSets(device, 1, &drawImageWrite, 0, nullptr);

		deletionQueue.PushFunction([&]()
		{
			globalDescriptorAllocator.DestroyPool(device);

			vkDestroyDescriptorSetLayout(device, drawImageDescriptorLayout, nullptr);
		});
	}

	void VulkanRenderer::InitPipelines()
	{
		InitBackgroundPipelines();
	}

	void VulkanRenderer::InitBackgroundPipelines()
	{
		VkPipelineLayoutCreateInfo computeLayout{};
		computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		computeLayout.pNext = nullptr;
		computeLayout.pSetLayouts = &drawImageDescriptorLayout;
		computeLayout.setLayoutCount = 1;

		VK_CHECK(vkCreatePipelineLayout(device, &computeLayout, nullptr, &gradientPipelineLayout));

		VkShaderModule computeDrawShader;
		if (!VulkanInitializers::LoadShaderModule(Pixey::Shaders::gradient_comp_spv, sizeof(Pixey::Shaders::gradient_comp_spv), device, &computeDrawShader))
		{
			Pixey::Log::Error("Error when building the compute shader");
		}

		VkPipelineShaderStageCreateInfo stageinfo{};
		stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageinfo.pNext = nullptr;
		stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageinfo.module = computeDrawShader;
		stageinfo.pName = "main";

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.pNext = nullptr;
		computePipelineCreateInfo.layout = gradientPipelineLayout;
		computePipelineCreateInfo.stage = stageinfo;

		VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &gradientPipeline));
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

		// Draw image size will match the window.
		VkExtent3D drawImageExtent =
		{
			windowExtent.width,
			windowExtent.height,
			1
		};

		// Hardcoding the draw format to 32 bit float.
		drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		drawImage.imageExtent = drawImageExtent;

		VkImageUsageFlags drawImageUsages{};
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
		drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		VkImageCreateInfo imageInfo = VulkanInitializers::ImageCreateInfo(drawImage.imageFormat, drawImageUsages, drawImageExtent);

		// For the draw image, we want to allocate it from gpu local memory.
		VmaAllocationCreateInfo imageAllocInfo = {};
		imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		imageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Allocate and create the image.
		vmaCreateImage(allocator, &imageInfo, &imageAllocInfo, &drawImage.image, &drawImage.allocation, nullptr);

		// Build an image view for the draw image to use for rendering.
		VkImageViewCreateInfo imageViewInfo = VulkanInitializers::ImageViewCreateInfo(drawImage.imageFormat, drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

		VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &drawImage.imageView));

		deletionQueue.PushFunction([this]()
		{
			vkDestroyImageView(device, drawImage.imageView, nullptr);
			vmaDestroyImage(allocator, drawImage.image, drawImage.allocation);
		});
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
