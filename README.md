# AgosGE
An open source Vulkan engine

## License : Apache 2.0
see below
https://www.apache.org/licenses/LICENSE-2.0.html

## Installation

### Requirements
- [CMake](https://cmake.org/download/)
- GCC (Linux)
- [Visual Studio](https://visualstudio.microsoft.com/) (Windows)
- [LunarG (Vulkan SDK)](https://vulkan.lunarg.com/sdk/home)

Note for Debian Users: Agos will download every required packages automaticly (see scripts/build_linux.sh)

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

Note: You can ignore the warnings that might appear

### macOS
Not supported yet