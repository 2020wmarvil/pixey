#pragma once

#if defined(_WIN32)
	#if defined(PIXEY_EXPORTS)
		#define PIXEY_API __declspec(dllexport)
	#else
		#define PIXEY_API __declspec(dllimport)
	#endif
#else
	#define PIXEY_API
#endif
