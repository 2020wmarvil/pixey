#pragma once

#include "VulkanTypes.h"

namespace Pixey::VulkanInitializers
{
	bool LoadShaderModule(const uint8_t* code, size_t codeSize, VkDevice device, VkShaderModule* outShaderModule);
}
