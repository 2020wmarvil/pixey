#include "Pixey/EntryPoint.h"

class PixeySample : public Pixey::App
{
public:
	void OnInit() override
	{
		Pixey::WindowManager::Get().OpenWindow("Pixey Sample", 1920, 1080);
	}

	void OnFrame(float deltaTime) override
	{
		(void)deltaTime;

		if (Pixey::Input::Get().IsKeyPressed(SDL_SCANCODE_ESCAPE))
		{
			Quit();
		}
	}
	void OnShutdown() override {}
};

PIXEY_RUN(PixeySample)
