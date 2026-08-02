#include "Pixey/App.h"

#include "Input.h"
#include "Vulkan/VulkanEngine.h"
#include "WindowManager.h"

#include <SDL3/SDL.h>

namespace Pixey
{
	int RunApp(App& inApp, int /*argc*/, char* /*argv*/[])
	{
		WindowManager& windowManager = WindowManager::Get();
		windowManager.Initialize();

		Window& window = windowManager.OpenWindow(inApp.GetWindowTitle(), inApp.GetWindowWidth(), inApp.GetWindowHeight());

		VulkanEngine& engine = VulkanEngine::Get();
		engine.Initialize(&window);

		Input input;
		inApp.OnInit();

		bool bQuit = false;
		Uint64 lastTicks = SDL_GetTicks();

		while (!bQuit)
		{
			SDL_Event e;
			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_EVENT_QUIT)
				{
					bQuit = true;
				}

				windowManager.HandleEvent(e);
			}

			input.Update();

			if (window.IsMinimized())
			{
				// throttle the speed to avoid the endless spinning
				SDL_Delay(100);
				continue;
			}

			const Uint64 currentTicks = SDL_GetTicks();
			const float deltaTime = static_cast<float>(currentTicks - lastTicks) / 1000.0f;
			lastTicks = currentTicks;

			inApp.OnFrame(deltaTime);
			engine.Draw();
		}

		inApp.OnShutdown();
		engine.Shutdown();
		windowManager.Shutdown();

		return 0;
	}
}
