#include "Pixey/Log.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>

#include <SDL3/SDL.h>

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

		std::ofstream logFile;
	}

	void Init()
	{
		const char* basePathRaw = SDL_GetBasePath();
		std::filesystem::path basePath = basePathRaw ? basePathRaw : std::filesystem::path(".");

		std::filesystem::path logsDir = basePath / "Logs";
		std::filesystem::create_directories(logsDir);

		std::filesystem::path currentLogPath = logsDir / "Pixey.log";

		if (std::filesystem::exists(currentLogPath))
		{
			std::filesystem::path backupDir = logsDir / "Backup";
			std::filesystem::create_directories(backupDir);

			const std::time_t nowTimeT = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm localTm{};
#ifdef _WIN32
			_tzset();
			localtime_s(&localTm, &nowTimeT);
#else
			localtime_r(&nowTimeT, &localTm);
#endif
			const std::string timestamp = std::format("{:04}-{:02}-{:02}_{:02}-{:02}-{:02}",
				localTm.tm_year + 1900, localTm.tm_mon + 1, localTm.tm_mday,
				localTm.tm_hour, localTm.tm_min, localTm.tm_sec);

			std::filesystem::rename(currentLogPath, backupDir / std::format("Pixey_{}.log", timestamp));
		}

		logFile.open(currentLogPath, std::ios::out | std::ios::trunc);
	}

	void Shutdown()
	{
		if (logFile.is_open())
		{
			logFile.close();
		}
	}

	void Write(Level level, std::string_view message)
	{
		std::string line = std::format("{}{}\n", Prefix(level), message);

		std::fputs(line.c_str(), level == Level::Error ? stderr : stdout);

		if (logFile.is_open())
		{
			logFile << line;
			logFile.flush();
		}

#ifdef _WIN32
		OutputDebugStringA(line.c_str());
#endif
	}
}
