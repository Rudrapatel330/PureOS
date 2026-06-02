@echo off
if not exist build\litehtml mkdir build\litehtml

set CFLAGS=-ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wextra -Wno-implicit-function-declaration -Wno-int-conversion -Wno-incompatible-pointer-types -mcmodel=large
set CXXFLAGS=%CFLAGS% -fno-exceptions -fno-rtti -fpermissive -DEA_COMPILER_NO_STANDARD_CPP_LIBRARY=1 -D__linux__=1
set INCLUDES=-Isrc\lib\std_bridge -Iinclude -Isrc\lib\EASTL\include -Isrc\lib\EABase\include\Common -Isrc\lib\litehtml\include -Isrc\lib\litehtml\include\litehtml -Isrc\lib\litehtml\src -Isrc\lib\litehtml\src\gumbo\include -Isrc\lib\litehtml\src\gumbo -Isrc\lib\litehtml\src\gumbo\include\gumbo

echo Compiling Gumbo (HTML Parser)...
for %%f in (src\lib\litehtml\src\gumbo\*.c) do (
    echo Compiling %%f...
    .\tools\bin\x86_64-elf-gcc.exe %CFLAGS% %INCLUDES% -c %%f -o build\litehtml\%%~nf.o
    if errorlevel 1 exit /b 1
)

echo Compiling Litehtml (CSS/Layout Engine)...
for %%f in (src\lib\litehtml\src\*.cpp) do (
    echo Compiling %%f...
    .\tools\bin\x86_64-elf-g++.exe %CXXFLAGS% %INCLUDES% -c %%f -o build\litehtml\%%~nf.o
    if errorlevel 1 exit /b 1
)

echo Archiving Litehtml...
if exist build\litehtml.a del build\litehtml.a
for %%f in (build\litehtml\*.o) do (
    .\tools\bin\x86_64-elf-ar.exe rcs build\litehtml.a %%f
)
if errorlevel 1 exit /b 1

echo Litehtml built successfully as build\litehtml.a!
