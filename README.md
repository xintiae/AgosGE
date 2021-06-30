# AgosGE
An open source Vulkan engine

## License : Apache 2.0
see below
https://www.apache.org/licenses/LICENSE-2.0.html

## Installation

### Requirements
- [CMake](https://cmake.org/download/)
- GCC (Linux) / [Visual Studio](https://visualstudio.microsoft.com/) (Windows)
- [LunarG (Vulkan SDK)](https://vulkan.lunarg.com/sdk/home)

### Windows
1. Download the required programs
2. Clone the repository: ```git clone https://github.com/xintiae/AgosGE.git --recursive```
3. Run *scripts/build_windows.bat*
4. Run the AgosGE.exe file in build/bin

### Linux
1. Run scripts/build_linux.sh
2. Create a new directory (eg. build)
3. cd into this directory
4. Run ```cmake ..``` and ```make && make install```
5. Open the bin/AgosGE executable

Note: You can ignore the warnings that might appear

### macOS
Not yet supported