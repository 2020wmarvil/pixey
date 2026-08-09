#include "VulkanPipelines.h"

#include "VulkanInitializers.h"

namespace Pixey::VulkanInitializers
{
	bool LoadShaderModule(const uint8_t* code, size_t codeSize, VkDevice device, VkShaderModule* outShaderModule)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = codeSize;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			return false;
		}

		*outShaderModule = shaderModule;
		return true;
	}
}
