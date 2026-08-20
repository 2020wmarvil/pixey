#include "UserInterface.h"

#include "Pixey/Window.h"
#include "Vulkan/VulkanInitializers.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

namespace Pixey
{
	UserInterface& UserInterface::Get()
	{
		static UserInterface instance;
		return instance;
	}

	void UserInterface::Init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t queueFamily, VkQueue queue, VkFormat colorFormat, Window& window)
	{
		ImGui::CreateContext();
		ImGui_ImplSDL3_InitForVulkan(window.GetHandle());

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = instance;
		initInfo.PhysicalDevice = physicalDevice;
		initInfo.Device = device;
		initInfo.QueueFamily = queueFamily;
		initInfo.Queue = queue;
		initInfo.DescriptorPoolSize = 64;
		initInfo.MinImageCount = 3;
		initInfo.ImageCount = 3;
		initInfo.UseDynamicRendering = true;
		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;

		ImGui_ImplVulkan_Init(&initInfo);

		bInitialized = true;
	}

	void UserInterface::Shutdown()
	{
		if (!bInitialized)
		{
			return;
		}

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		bInitialized = false;
	}

	void UserInterface::ProcessEvent(const SDL_Event& event)
	{
		if (!bInitialized)
		{
			return;
		}

		ImGui_ImplSDL3_ProcessEvent(&event);
	}

	void UserInterface::NewFrame()
	{
		if (!bInitialized)
		{
			return;
		}

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}

	void UserInterface::Render(VkCommandBuffer commandBuffer, VkImageView targetImageView, VkExtent2D extent)
	{
		ImGui::Render();

		VkRenderingAttachmentInfo colorAttachment = VulkanInitializers::AttachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingInfo renderInfo = VulkanInitializers::RenderingInfo(extent, &colorAttachment, nullptr);

		vkCmdBeginRendering(commandBuffer, &renderInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
		vkCmdEndRendering(commandBuffer);
	}
}
