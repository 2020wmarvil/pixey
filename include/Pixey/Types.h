#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace Pixey
{
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Mat4 = glm::mat4;

	struct Color
	{
		float R, G, B, A;
	};

	struct Rect
	{
		float X, Y, W, H;
	};
}
