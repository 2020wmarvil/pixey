#include "Pixey/EntryPoint.h"

class Sample : public Pixey::App
{
public:
	void OnInit() override {}
	void OnFrame(float deltaTime) override { (void)deltaTime; }
	void OnShutdown() override {}
};

PIXEY_RUN(Sample)
