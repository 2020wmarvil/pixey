#pragma once

namespace Pixey
{
	class App
	{
	public:
		virtual ~App() = default;

		virtual const char* GetWindowTitle() const { return "Pixey"; }
		virtual int GetWindowWidth() const { return 1920; }
		virtual int GetWindowHeight() const { return 1080; }

		virtual void OnInit() {}
		virtual void OnFrame(float deltaTime) { (void)deltaTime; }
		virtual void OnShutdown() {}
	};

	int RunApp(App& inApp, int argc, char* argv[]);
}
