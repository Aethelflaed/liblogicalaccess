cd ..
call scripts/cleancmake.bat
cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET_ARCH=x64 -G"Visual Studio 11 Win64" -Tv110_xp .
pause;