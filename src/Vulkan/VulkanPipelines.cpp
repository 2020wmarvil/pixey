#include "VulkanPipelines.h"

#include "VulkanInitializers.h"

#include <fstream>

namespace Pixey::VulkanInitializers
{
	bool LoadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule)
	{
		// Open the file.
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			return false;
		}

		size_t fileSize = (size_t)file.tellg();

		// spirv expects the buffer to be on uint32, so make sure to reserve a int vector big enough for the entire file.
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);
		file.close();

		// Create a new shader module, using the buffer we loaded.
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = buffer.size() * sizeof(uint32_t);
		createInfo.pCode = buffer.data();

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			return false;
		}

		*outShaderModule = shaderModule;
		return true;
	}
}
