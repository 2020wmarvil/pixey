#include "Pixey/EntryPoint.h"

class PixeySample : public Pixey::App
{
public:
	const char* GetWindowTitle() const override { return "Pixey Sample"; }

	void OnInit() override {}
	void OnFrame(float deltaTime) override { (void)deltaTime; }
	void OnShutdown() override {}
};

PIXEY_RUN(PixeySample)
