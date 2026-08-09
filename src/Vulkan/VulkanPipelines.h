#pragma once

#include "VulkanTypes.h"

namespace Pixey::VulkanInitializers
{
	bool LoadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule);
}
