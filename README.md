# AgosGE
An open source Vulkan-based game engine (WIP).

## License : Apache 2.0
See https://www.apache.org/licenses/LICENSE-2.0.html

## Installation

### Requirements
- [CMake](https://cmake.org/download/)
- GCC (Linux)
- [Visual Studio](https://visualstudio.microsoft.com/) (Windows)
- [LunarG (Vulkan SDK)](https://vulkan.lunarg.com/sdk/home)

*Note for Debian Users*: Agos will download every required packages automaticly (see [scripts/build_linux.sh](https://github.com/xintiae/AgosGE/blob/main/scripts/build_linux.sh))

### Windows
1. Download the required programs
2. Clone the repository: ```git clone https://github.com/xintiae/AgosGE.git --recursive```
3. Run *scripts/build_windows.bat*
4. Run the AgosGE.exe file in build/bin

### Linux
- AgosGE's mainly developped and maintained on Debian-based linux distributions.
- **DO NOT** clone the github repository **RECURSIVELY** ! When building AgosGE, all dependencies and required packages will be downloaded automatically (see scripts/build_linux.sh for more details).
1. Clone the repository **without the recursive option** : ```git clone https://github.com/xintiae/AgosGE.git```
2. Create a build repo (e.g. "build"): ```mkdir build```
3. Cd in that repo : ```cd build```
4. Run ```cmake .. && make && ./bin/AgosGE```

### macOS
Not supported yet


## How to use AgosGE ?
WIP - Tutorials coming later up

## Credits and notes
- AgosGE is still in VERY HEAVY WIP phase
- Many utilities and helper functions regarding AgosGE's Graphics API Vulkan have been greatly inspired from **[Alexander Overvoorde](https://github.com/Overv)'s [Vulkan tutorial](https://vulkan-tutorial.com/)**.
- **Special thanks to all contributors** :
    *[Dodopriester](https://github.com/Dodopriester)*,
    *[Blackalpaul](https://github.com/Blackalpaul)*

*Notes:*
- AgosGE allows you to **choose** on which **graphics API AgosGE's Renderer** should be based. This enables you to implement your own AgosGE's extensions based on the API YOU want. In order to select which API you want, consider adding the following flag when running cmake : ```-DAGOS_GRAPHICS_API=*the_api_you_want*``` (see *How to use AgosGE? - section AgosGE's graphics API* (*for instance :* -DAGOS_GRAPHICS_API=Vulkan). At the moment, AgosGE **only supports Vulkan**.
- At the moment, AgosGE's supported graphics API are **glfw-based only APIs**. 

## EDIT - AgosGE's rework phase
AgosGE has undergone an intense 8-months rework phase since last update.
Older versions of AgosGE were bugs-filled, with poorly made source code (making development phases nightmares and bug fixes even worse).
This led to a complete code rework, which took us a considerable amount of time to do.
Now AgosGE's much easier to develop, features and bug fixes have become more and more easy to deal with.
