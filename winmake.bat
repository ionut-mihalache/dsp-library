@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64

if "%~1" == "" goto build
if /I "%~1" == "build" goto build
if /I "%~1" == "clean" goto clean


:build
    nmake /f Makefile.win
    goto :eof

:clean
    nmake /f Makefile.win clean
    goto :eof
