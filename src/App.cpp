#include "Pixey/App.h"

#include "Pixey/Input.h"
#include "Pixey/Log.h"
#include "Pixey/WindowManager.h"
#include "UserInterface.h"
#include "Vulkan/VulkanRenderer.h"

#include <SDL3/SDL.h>

namespace Pixey
{
	int RunApp(App& inApp, int /*argc*/, char* /*argv*/[])
	{
		Log::Init();
		Log::Info("Pixey engine starting");

		WindowManager& windowManager = WindowManager::Get();
		windowManager.Initialize();
		inApp.OnInit();

		Input& input = Input::Get();
		UserInterface& userInterface = UserInterface::Get();
		VulkanRenderer& renderer = VulkanRenderer::Get();

		bool bQuit = false;
		Uint64 lastTicks = SDL_GetTicks();

		while (!bQuit)
		{
			SDL_Event e;
			while (SDL_PollEvent(&e) != 0)
			{
				userInterface.ProcessEvent(e);

				if (e.type == SDL_EVENT_QUIT)
				{
					bQuit = true;
				}

				windowManager.HandleEvent(e);
			}

			input.Update();

			if (windowManager.AreAllWindowsMinimized())
			{
				// throttle the speed to avoid the endless spinning
				SDL_Delay(100);
				continue;
			}

			const Uint64 currentTicks = SDL_GetTicks();
			const float deltaTime = static_cast<float>(currentTicks - lastTicks) / 1000.0f;
			lastTicks = currentTicks;

			userInterface.NewFrame();
			inApp.OnFrame(deltaTime);
			renderer.Draw();

			if (inApp.WantsToQuit())
			{
				bQuit = true;
			}
		}

		inApp.OnShutdown();
		renderer.Shutdown();
		windowManager.Shutdown();

		Log::Info("Pixey engine shutting down");
		Log::Shutdown();

		return 0;
	}
}
