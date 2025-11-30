@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64

if "%~1" == "" (
    nmake /f Makefile
    goto :eof
)

if /I "%~1"=="clean" (
    nmake clean
    goto :eof
)
