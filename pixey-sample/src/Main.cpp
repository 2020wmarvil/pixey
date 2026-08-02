#include "Pixey/EntryPoint.h"

class PixeySample : public Pixey::App
{
public:
	void OnInit() override
	{
		Pixey::WindowManager::Get().OpenWindow("Pixey Sample", 1920, 1080);
	}

	void OnFrame(float deltaTime) override { (void)deltaTime; }
	void OnShutdown() override {}
};

PIXEY_RUN(PixeySample)
