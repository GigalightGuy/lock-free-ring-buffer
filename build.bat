@echo off

if "%~1"=="debug" goto debug else goto release

:debug
mkdir build\debug
call g++ main.cpp -o build\debug\main.exe -g

goto end

:release
mkdir build\release
call g++ main.cpp -o build\release\main.exe

goto end

:end
echo Build finished
