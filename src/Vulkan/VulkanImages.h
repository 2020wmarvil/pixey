#pragma once

#include <vulkan/vulkan.h>

namespace Pixey::VulkanUtils
{
	void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
}
