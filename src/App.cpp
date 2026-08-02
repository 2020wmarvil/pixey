#include "Pixey/App.h"

#include "Vulkan/VulkanEngine.h"

namespace Pixey
{
	int RunApp(App& inApp, int /*argc*/, char* /*argv*/[])
	{

		VulkanEngine engine;
		engine.Initialize();
		engine.Run();
		engine.Shutdown();




		inApp.OnInit();
		// TODO: open SDL window, init Vulkan, run frame loop.
		inApp.OnShutdown();
		return 0;
	}
}
