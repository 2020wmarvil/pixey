#include "Pixey/App.h"

namespace Pixey
{
	int RunApp(App& InApp, int /*Argc*/, char* /*Argv*/[])
	{
		InApp.OnInit();
		// TODO: open SDL window, init Vulkan, run frame loop.
		InApp.OnShutdown();
		return 0;
	}
}
