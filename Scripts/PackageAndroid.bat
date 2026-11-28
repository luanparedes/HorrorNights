@echo off
REM Packages the project for Android without opening the editor.
REM Equivalent to: Platforms > Android > Package Project (output: Packages\Android_<FLAVOR>)
REM Usage:
REM   PackageAndroid.bat                   -> ASTC, Development
REM   PackageAndroid.bat ETC2              -> another texture flavor (ASTC, ETC2, DXT, Multi)
REM   PackageAndroid.bat ASTC Shipping     -> flavor + configuration
REM Then run InstallAndroid.bat to install it on the phone.
REM Engine: set UE_ROOT to use a different Unreal installation.

setlocal
set "FLAVOR=%~1"
if "%FLAVOR%"=="" set "FLAVOR=ASTC"
set "CONFIG=%~2"
if "%CONFIG%"=="" set "CONFIG=Development"
if not defined UE_ROOT set "UE_ROOT=C:\Users\luans\Epic Games\UE_5.8"

for %%i in ("%~dp0..") do set "PROJECT_DIR=%%~fi"
set "PROJECT=%PROJECT_DIR%\HorrorNights.uproject"
set "ARCHIVE_DIR=%PROJECT_DIR%\Packages\Android_%FLAVOR%"
set "UAT=%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

if not exist "%UAT%" (
    echo [ERROR] RunUAT.bat not found at: %UAT%
    echo Set UE_ROOT to your Unreal Engine 5.8 folder.
    goto :fail
)

REM The editor (Live Coding) blocks C++ builds from outside it
tasklist /FI "IMAGENAME eq UnrealEditor.exe" 2>nul | find /I "UnrealEditor.exe" >nul && (
    echo [ERROR] Unreal Editor is open.
    echo Close the editor before packaging ^(Live Coding blocks the C++ build^).
    goto :fail
)

echo Packaging HorrorNights for Android ^(%FLAVOR%, %CONFIG%^)...
echo Output: %ARCHIVE_DIR%
echo.

call "%UAT%" BuildCookRun -nop4 -utf8output ^
 -project="%PROJECT%" -target=HorrorNights ^
 -unrealexe="%EDITOR_CMD%" ^
 -platform=Android -cookflavor=%FLAVOR% -clientconfig=%CONFIG% ^
 -installed -zenstore -SkipCookingErrorSummary ^
 -build -cook -stage -package -pak -iostore -compressed ^
 -archive -archivedirectory="%ARCHIVE_DIR%" ^
 -nocompileuat
if errorlevel 1 goto :fail

echo.
echo [OK] Build ready at %ARCHIVE_DIR%
echo To install on the phone: InstallAndroid.bat %FLAVOR%
endlocal
pause
exit /b 0

:fail
echo.
echo [ERROR] Packaging failed. See the log above.
endlocal
pause
exit /b 1
