@echo off
setlocal

rem File: <root>\Scripts\vs22\GenerateProjects.bat
rem Ascend from Scripts\vs22 to project root, independent of launch directory.
for %%I in ("%~dp0..\..") do set "ROOT_DIR=%%~fI"
set "PREMAKE=%ROOT_DIR%\vendor\premake\premake5.exe"

if not exist "%PREMAKE%" (
    echo [ERROR] Premake executable not found: "%PREMAKE%"
    exit /b 1
)
if not exist "%ROOT_DIR%\premake5.lua" (
    echo [ERROR] Workspace file not found: "%ROOT_DIR%\premake5.lua"
    exit /b 1
)

pushd "%ROOT_DIR%" || exit /b 1

echo ========================================
echo Generating plagc Visual Studio 2022 files
echo ========================================
"%PREMAKE%" vs2022
set "RESULT=%ERRORLEVEL%"

popd

if not "%RESULT%"=="0" (
    echo [ERROR] Premake generation failed with exit code %RESULT%.
    exit /b %RESULT%
)

echo [SUCCESS] Visual Studio 2022 files generated.
exit /b 0
