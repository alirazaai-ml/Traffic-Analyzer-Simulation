# Traffic Analyzer

## Project Overview

Traffic Analyzer is a simple C++ application that models a road network as a graph of `Node`s and `Edge`s and provides a 2D visualization using the SFML (Simple and Fast Multimedia Library) framework. The renderer is implemented in `MapRenderer.cpp`/`MapRenderer.h` and uses SFML to draw nodes, edges and highlighted routes. Graph data is kept in `Graph.cpp`/`Graph.h`. The project may also include modules that compute or predict traffic levels (for example a `PredictionSystem`).

From the code in `MapRenderer.cpp` you can see the renderer expects:
- `Node` with fields `id`, `x`, `y`.
- `Edge` with fields `fromNodeId`, `toNodeId`, `trafficLevel`, `name`.
- `Graph` with methods `getNode(int id)`, `getAllNodes()` and `getAllEdges()`.

Typical application flow:
1. Load or build the `Graph` data structure (nodes and edges).
2. Create an SFML `sf::RenderWindow` and a `MapRenderer` instance.
3. In the main loop call `MapRenderer::drawGraph(...)` to render the map; optionally call `drawRoute(...)` for a computed route.
4. The renderer uses an `arial.ttf` font file by default; ensure a font file is available alongside the executable or change the path.


## Files (example)
- `MapRenderer.cpp`, `MapRenderer.h` - rendering logic using SFML.
- `Graph.cpp`, `Graph.h` - graph storage and lookup.
- `main.cpp` - application entry point and game loop (may be named differently).
- `PredictionSystem.*` - traffic prediction logic (optional).
- `arial.ttf` - font used by the renderer (should be placed in the working directory at runtime).


## Dependencies
- C++ compiler (supporting at least C++11/C++17).
- SFML 2.5 or newer (graphics, window, system modules).
- On Windows: Visual Studio 2017/2019/2022 or MinGW toolchain.


## How SFML is used in this project
This project uses SFML for window creation and 2D drawing primitives:
- `sf::RenderWindow` for the main window and the render loop.
- `sf::Vertex` and `sf::Lines` for drawing edges and routes.
- `sf::CircleShape` for nodes and `sf::Text` for labels.
- `sf::Font` to load `arial.ttf` and render text.


## How to add SFML to this project (Windows / Visual Studio)
Below are step-by-step guides for the most common workflows.

A) Manual download and Visual Studio configuration
1. Download SFML from https://www.sfml-dev.org/download.php (choose the version matching your compiler). Extract it.
2. In Visual Studio open the project `Traffic Analyzer.vcxproj`.
3. Project -> Properties -> C/C++ -> General -> Additional Include Directories: add the SFML `include` folder (e.g. `C:\path\to\SFML-2.5.1\include`).
4. Project -> Properties -> Linker -> General -> Additional Library Directories: add the SFML `lib` folder (e.g. `C:\path\to\SFML-2.5.1\lib`).
5. Project -> Properties -> Linker -> Input -> Additional Dependencies:
   - For Release: `sfml-graphics.lib; sfml-window.lib; sfml-system.lib` (plus other modules you use).
   - For Debug: append `-d` to library names: `sfml-graphics-d.lib; sfml-window-d.lib; sfml-system-d.lib`.
6. Copy the required DLLs from the SFML `bin` folder into your executable output directory (e.g. `sfml-graphics-2.dll`, `sfml-window-2.dll`, `sfml-system-2.dll`) or ensure they are in PATH.
7. Make sure Runtime Library settings match SFML builds (Multi-threaded DLL (/MD) vs static). Using the prebuilt SFML, use dynamic linking (/MD) by default.


B) Using vcpkg (recommended for convenience)
1. Install and bootstrap `vcpkg` following https://github.com/microsoft/vcpkg.
2. Install SFML: `vcpkg install sfml:x64-windows` (or `x86-windows`).
3. Integrate with Visual Studio: `vcpkg integrate install`.
4. The project will automatically find SFML headers and link libraries when building in Visual Studio.


C) Using CMake (cross-platform)
If you prefer CMake, add a small snippet to your `CMakeLists.txt`:

```cmake
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
add_executable(TrafficAnalyzer main.cpp MapRenderer.cpp Graph.cpp)
target_include_directories(TrafficAnalyzer PRIVATE ${SFML_INCLUDE_DIRS})
target_link_libraries(TrafficAnalyzer PRIVATE sfml-graphics sfml-window sfml-system)
```

Notes:
- On Windows you may need to set `SFML_STATIC` and link the `-s` or `-d` static libraries if building SFML statically.
- With vcpkg you can configure CMake to use the vcpkg toolchain file: `cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`.


## Building from command line (g++ example on Linux)
Assuming SFML is installed system-wide:

```bash
g++ -std=c++17 main.cpp MapRenderer.cpp Graph.cpp -I/usr/include -L/usr/lib -lsfml-graphics -lsfml-window -lsfml-system -o TrafficAnalyzer
```


## Running the program
- Ensure `arial.ttf` (or the font you use) is present in the same folder as the executable or change the font path in `MapRenderer::MapRenderer()` / `loadFont()`.
- If using dynamic SFML linking on Windows, copy the SFML DLLs into the executable folder.
- Run the executable; the main loop should open a window and draw the loaded graph.


## Troubleshooting
- "Missing DLL" at startup: copy SFML DLLs from SFML `bin` to the executable folder.
- Linker errors: check that you linked the correct SFML libraries and that Debug/Release uses `-d` suffixed libraries appropriately.
- Runtime crashes when drawing text: verify the font file exists and the working directory is correct.
- Header not found: add the SFML `include` directory to the project's additional include directories.


## How you can contribute / extend
- Add a proper `main.cpp` that parses map data and runs the renderer.
- Add data importers for common map formats (CSV, OSM, etc.).
- Implement or refine `PredictionSystem` to compute dynamic `trafficLevel` for edges.
- Improve UI controls (zoom, pan, select nodes) and add on-screen information overlays.


If you want, I can also:
- Generate a `CMakeLists.txt` that compiles the current source files.
- Add a `main.cpp` skeleton that opens an SFML window and shows the graph.


---
