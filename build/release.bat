@echo off

:: ----------------------------------------------------

set curr_dir=%cd%
set base_dir=%curr_dir%/..
set install_dir=%base_dir%/install
set build_dir=win-x86_64
set extern_dir=%base_dir%/extern

:: ----------------------------------------------------

set cmake_generator=Visual Studio 16 2019
set cmake_build_type=Release
set cmake_config=Release
set debug_flag=
set verbose_flag=

:: ----------------------------------------------------

:: Iterate over command line arguments
:argsloop

  if "%1" == "" (
    goto :argsloopdone
  )

  if "%1" == "verbose" (
    set verbose_flag=/verbosity:diagnostic /fl
  )

  if "%1" == "debug" (
   set debug_flag=-debug
   set cmake_build_type=Debug
   set cmake_config=Debug
   set build_dir=win-x86_64-debug
  )

shift
goto :argsloop
:argsloopdone

:: ----------------------------------------------------

echo CMake Build Type: %cmake_build_type%
echo CMake Config: %cmake_config%
echo Build dir: %build_dir%

:: ----------------------------------------------------

if not exist "%curr_dir%/%build_dir%" mkdir "%curr_dir%/%build_dir%"
if not exist "%install_dir%" mkdir "%install_dir%"

:: ----------------------------------------------------

cd "%curr_dir%/%build_dir%"

@echo on
cmake -G "%cmake_generator%" ^
      -A X64 ^
      -DCMAKE_INSTALL_PREFIX="%install_dir%" ^
      -DCMAKE_VERBOSE_MAKEFILE=On ^
      -DCMAKE_BUILD_TYPE="%cmake_build_type%" ^
      ..

echo Done
if errorlevel 1 goto err

:: ----------------------------------------------------

cmake --build . ^
      --target install ^
      --config "%cmake_config%" ^
      --parallel 10 ^
      -- %verbose_flag%

if errorlevel 1 goto err

:: ----------------------------------------------------

cd "%install_dir%"
cd bin

:: ----------------------------------------------------
test-shared-gl-context.exe
:: test-shared-gl-context-with-fbo.exe
:: ----------------------------------------------------

cd "%curr_dir%"
goto eof

:err
echo An error occured. See above log for more info

:eof
cd "%curr_dir%"

