# AgosGE
An open source Vulkan-based game engine.

## License : Apache 2.0
see below
https://www.apache.org/licenses/LICENSE-2.0.html

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
1. Clone the repository: ```git clone https://github.com/xintiae/AgosGE.git```
2. Create a build repo (e.g. "build"): ```mkdir build```
3. Cd in that repo : ```cd build```
4. Run ```cmake .. && make && ./bin/AgosGE```

### macOS
Not supported yet


## Credits and notes
- A **huge thanks to [Alexander Overvoorde](https://github.com/Overv) for his amazing** [Vulkan tutorial](https://vulkan-tutorial.com/), without which we surely wouldn't have been so far (also make sure to checkout his [github's tutorial page](https://github.com/Overv/VulkanTutorial))!
- Also **huge thanks too to [Joey de Vries](https://joeydevries.com/) for his amazing** [OpenGL tutorial](https://github.com/JoeyDeVries/LearnOpenGL/), because, you know, everything's been learnt on the stack :>!
- **Special thanks to all contributors** : 
    *[Dodopriester](https://github.com/Dodopriester)*,
    *[Blackalpaul](https://github.com/Blackalpaul)*

*Notes:*
- AgosGE allows you to **choose** on which **graphics API AgosGE's Renderer** should be based. This enables you to implement your own AgosGE's extensions based on the API YOU want :). In order to select which API you want, consider adding the following flag when running cmake : ```-DAGOS_GRAPHICS_API=*the_api_you_want*```. At the moment, AgosGE **only supports Vulkan**.
- Each AgosGE's supported graphics API are **glfw-based only APIs**. 
- (You can ignore the warnings that might appear when compiling...?)

