@echo off
setlocal EnableExtensions DisableDelayedExpansion

rem Place this script in the root of your Git repository.
cd /d "%~dp0" || exit /b 1

echo ==== Current Git status ====
git status || goto :error

echo.
set /p "continue=Press Enter to stage all changes..."

git add -A || goto :error

echo.
set "commitMessage="
set /p "commitMessage=Enter commit message: "
if not defined commitMessage (
    echo [ERROR] Commit message cannot be empty.
    exit /b 1
)

git commit -m "%commitMessage%" || goto :error
git push origin main || goto :error

echo.
echo [SUCCESS] Changes pushed to origin/main.
exit /b 0

:error
echo.
echo [ERROR] Git command failed. Push was not completed.
exit /b 1
