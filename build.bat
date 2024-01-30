setlocal

cd /d %~dp0
mkdir vsbuild
cd vsbuild
cmake -G "Visual Studio 16 2019" .. -A Win32
cmake --build . --config Release

endlocal