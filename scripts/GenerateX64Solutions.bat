call scripts\cleancmake.bat
cmake.exe -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH=x64 -G"Visual Studio 11 Win64" -Tv110_xp . -DCMAKE_INSTALL_PREFIX=./install .