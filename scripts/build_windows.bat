@ECHO OFF

echo Updating submodules...
git submodule init
git submodule update

FOR /F "tokens=* USEBACKQ" %%F IN (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) DO (
SET MSBuildPath=%%F
)

echo Creating Visual Studio Files...
cd ..
if not exist build mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
echo Building AgosGE...
"%MSBuildPath%" AgosGE.sln

pause