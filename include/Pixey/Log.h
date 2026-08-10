#pragma once

#include "Pixey/PixeyExport.h"

#include <format>
#include <string_view>
#include <utility>

namespace Pixey::Log
{
	enum class Level
	{
		Info,
		Warning,
		Error,
	};

	void Init();
	void Shutdown();

	void Write(Level level, std::string_view message);

	template<typename... Args>
	void Info(std::format_string<Args...> fmt, Args&&... args)
	{
		Write(Level::Info, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void Warning(std::format_string<Args...> fmt, Args&&... args)
	{
		Write(Level::Warning, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void Error(std::format_string<Args...> fmt, Args&&... args)
	{
		Write(Level::Error, std::format(fmt, std::forward<Args>(args)...));
	}
}
