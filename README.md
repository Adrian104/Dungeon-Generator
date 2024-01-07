![](https://github.com/Adrian104/Dungeon-Generator/blob/master/resources/logo.png)
---
![](https://img.shields.io/github/license/Adrian104/Dungeon-Generator?color=blue)
![](https://img.shields.io/github/languages/top/Adrian104/Dungeon-Generator?color=blue)
### Dungeon-Generator is a free and open-source BSP-based procedural 2D map generator, written in C++.

# :star: Features
* Various settings allow you to precisely customize the appearance of the dungeon.
* Generator receives seed value, so the outcome is always predictable.
* Rooms can be composed of two rectangular surfaces, creating unique structures.
* Algorithm can reduce room density in some areas, thus ensuring output looks more realistic.
* And a lot more!

# :bulb: Example usage
```C++
#include <dgen/dgen.hpp>

int main()
{
    dg::Input input = dg::GetExampleInput();
    dg::Output output{};

    // In reality, you should set input member variables to your liking
    // and not depend on dg::GetExampleInput().

    dg::Generate(&input, &output);

    // Geometry of a dungeon is now generated, it should be processed
    // further (e.g., converting it to a tile map, postprocessing).

    return 0;
}
```

The generator fills object of type `dg::Output` with geometry data
based on the data from object of type `dg::Input`. The output contains
coordinates and dimensions of different dungeon structures (e.g., rooms, paths).

# :gear: How does it work?
![](https://github.com/Adrian104/Dungeon-Generator/blob/master/resources/animation.gif)
---
Function `dg::Generate()` performs internally several steps:
1. Algorithm recursively divides entire space into smaller cells, keeping the parent-cells
in memory. This method is known as BSP, which produces binary-tree structure. In 
addition, leaf cells create `Tag` objects at the corners of them.
2. In some cells, `Room` objects are placed. Here `Tag` objects are also placed,
but this time, on the room entrance axes, in between cells.
3. Algorithm creates `Vertex` objects based on `Tag` objects. Multiple tags are
combined into one `Vertex` and all resulting vertices are linked together with
pointers. To do this step, algorithm sorts `Tag` objects beforehand, based on their positions.
4. Previously created BSP-tree is traversed postorder, recursively connecting 
`Room` objects by searching path between them, using A* algorithm.
5. At this point, special method optimizes `Vertex` objects, based on created paths.
6. Generator produces output data.

# :mag: What is included in this repository?
Dungeon-Generator project consists of several sub-projects:
* `dgen` - generator library itself. Has no dependencies other than STL.
* `dgen-app` - application that uses `dgen` library. Requires **SDL2** and **SDL2_ttf**.
* `dgen-benchmark` - micro-benchmarking utility. Measures performance of the `dgen` library.

# :hammer_and_wrench: Building
This project can be built in two different ways: by using Visual Studio or by using CMake.

## Visual Studio
### Prerequisites:
* Git (only for cloning)
* Microsoft Visual Studio 2022
* vcpkg (for `dgen-app` - make sure **SDL2** and **SDL2_ttf** are installed)

### Steps:
1. Clone this repository (or download by clicking Code -> Download ZIP).
2. Open `Dungeon-Generator.sln`.
3. Select startup project (e.g. `dgen-app`) and compile.

Compiled executables are located in `build` directory.

## CMake
### Prerequisites:
* Git (only for cloning)
* Compiler that supports C++17
* CMake (version >= 3.21)

### Steps:
1. Clone this repository (or download by clicking Code -> Download ZIP).
2. Open terminal in `Dungeon-Generator` directory.
3. Run the following command:
```console
cmake -S . -B build && cmake --build build
```

CMake will detect if `Dungeon-Generator` is top level project.
If so, it will automatically download **SDL2** with **SDL2_ttf** and compile all targets.
Otherwise, only `dgen` target will be created.
Compiled executables are located in `build/bin` directory.

# :framed_picture: Images
### The following image shows a visual representation of what `dgen` library can produce:
![](https://github.com/Adrian104/Dungeon-Generator/blob/master/resources/geometry.png)
### This geometry can be easily transformed into a tile map¹. For example:
![](https://github.com/Adrian104/Dungeon-Generator/blob/master/resources/map.png)

¹Currently, the `dgen` library itself doesn't have this implemented yet.