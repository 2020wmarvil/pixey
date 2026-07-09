# Pixey

General purpose hobby game/rendering engine.

## Requirements

- Windows
- [Vulkan SDK](https://vulkan.lunarg.com/) 1.4.x
- CMake 3.28+
- Git

## Build

```pwsh
git clone <url> pixey
cd pixey
.\setup.bat
.\build\pixey-sample\Debug\pixey-sample.exe
```

On macOS / Linux, use `./setup.sh` in place of `.\setup.bat`.

## Using Pixey from a game

Pixey is intended to be consumed as a git submodule of a downstream game repo and pulled into that game's CMake via `add_subdirectory`.

```cmake
# In your game's CMakeLists.txt
add_subdirectory(third_party/pixey)
target_link_libraries(my-game PRIVATE pixey::pixey)
```

```cpp
// In your game's main.cpp - and ONLY this file, per executable
#include "Pixey/EntryPoint.h"

class MyGame : public Pixey::App
{
public:
    void OnInit() override {}
    void OnFrame(float DeltaTime) override {}
    void OnShutdown() override {}
};

PIXEY_RUN(MyGame)
```

## License

See [LICENSE](LICENSE).
