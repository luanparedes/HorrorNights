@echo off
REM Roda o build empacotado do Windows (Packages\Windows\HorrorNights.exe).
REM Uso:
REM   RunGame.bat            -> tela cheia
REM   RunGame.bat windowed   -> janela 960x540 (bom para testar varias instancias/LAN)
REM Argumentos extras sao repassados ao jogo (ex: RunGame.bat windowed -log)

setlocal
set "EXE=%~dp0..\Packages\Windows\HorrorNights.exe"

if not exist "%EXE%" (
    echo [ERRO] Executavel nao encontrado:
    echo   %EXE%
    echo Rode PackageWindows.bat para gerar o build.
    pause
    exit /b 1
)

if /I "%~1"=="windowed" (
    shift
    start "" "%EXE%" -WINDOWED -ResX=960 -ResY=540 %1 %2 %3 %4 %5 %6 %7 %8 %9
) else (
    start "" "%EXE%" -FULLSCREEN %*
)
endlocal
