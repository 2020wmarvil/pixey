#include "Pixey/Log.h"

#include <cstdio>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Pixey::Log
{
	namespace
	{
		const char* Prefix(Level level)
		{
			switch (level)
			{
			case Level::Info:    return "[Info] ";
			case Level::Warning: return "[Warning] ";
			case Level::Error:   return "[Error] ";
			}
			return "";
		}
	}

	void Write(Level level, std::string_view message)
	{
		std::string line = std::format("{}{}\n", Prefix(level), message);

		std::fputs(line.c_str(), level == Level::Error ? stderr : stdout);

#ifdef _WIN32
		OutputDebugStringA(line.c_str());
#endif
	}
}
