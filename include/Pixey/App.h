#pragma once

namespace Pixey
{
	class App
	{
	public:
		virtual ~App() = default;

		virtual void OnInit() {}
		virtual void OnFrame(float deltaTime) { (void)deltaTime; }
		virtual void OnShutdown() {}

		bool WantsToQuit() const { return bWantsToQuit; }

	protected:
		void Quit() { bWantsToQuit = true; }

	private:
		bool bWantsToQuit = false;
	};

	int RunApp(App& inApp, int argc, char* argv[]);
}
