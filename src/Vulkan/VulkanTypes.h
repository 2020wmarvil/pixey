#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#include <cstdio>
#include <format>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#define VK_CHECK(x)                                                              \
    do {                                                                         \
        VkResult err = x;                                                        \
        if (err) {                                                               \
            std::fputs(std::format("Detected Vulkan error: {}\n",                \
                string_VkResult(err)).c_str(), stderr);                          \
            abort();                                                             \
        }                                                                        \
    } while (0)
