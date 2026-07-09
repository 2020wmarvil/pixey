#pragma once

namespace Pixey
{
	class App
	{
	public:
		virtual ~App() = default;

		virtual void OnInit() {}
		virtual void OnFrame(float DeltaTime) { (void)DeltaTime; }
		virtual void OnShutdown() {}
	};

	int RunApp(App& InApp, int Argc, char* Argv[]);
}
