@echo off
setlocal EnableExtensions

rem File: <root>\Scripts\vs22\GenerateProjects.bat
rem Generate only Visual Studio 2022 solution/project files.
rem Dependencies must already exist under <root>\third-party.
for %%I in ("%~dp0..\..") do set "ROOT_DIR=%%~fI"
set "PREMAKE=%ROOT_DIR%\third-party\premake\premake5.exe"

if not exist "%PREMAKE%" (
    echo [ERROR] Premake executable not found: "%PREMAKE%"
    exit /b 1
)

pushd "%ROOT_DIR%" || exit /b 1
"%PREMAKE%" vs2022
set "RESULT=%ERRORLEVEL%"
popd

exit /b %RESULT%
