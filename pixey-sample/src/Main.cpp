#include "Pixey/EntryPoint.h"

class Sample : public Pixey::App
{
public:
	void OnInit() override {}
	void OnFrame(float DeltaTime) override { (void)DeltaTime; }
	void OnShutdown() override {}
};

PIXEY_RUN(Sample)
