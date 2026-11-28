@echo off
REM Instala o build Android (Packages\Android_ASTC) no celular conectado via USB.
REM Requisitos: depuracao USB ativada no celular e adb (Android SDK platform-tools).
REM Uso:
REM   InstallAndroid.bat          -> instala o build ASTC
REM   InstallAndroid.bat ETC2     -> instala outro flavor (Packages\Android_ETC2)

setlocal EnableDelayedExpansion
set "FLAVOR=%~1"
if "%FLAVOR%"=="" set "FLAVOR=ASTC"
set "BUILD_DIR=%~dp0..\Packages\Android_%FLAVOR%"
set "PACKAGE=com.SunnymoonSoftwares.HorrorNights"

REM Localiza o adb
set "ADB=adb"
if defined ANDROID_HOME if exist "%ANDROID_HOME%\platform-tools\adb.exe" set "ADB=%ANDROID_HOME%\platform-tools\adb.exe"
"%ADB%" version >nul 2>&1 || (
    echo [ERRO] adb nao encontrado. Instale o Android SDK platform-tools ou defina ANDROID_HOME.
    goto :fail
)

if not exist "%BUILD_DIR%" (
    echo [ERRO] Pasta de build nao encontrada: %BUILD_DIR%
    echo Rode PackageAndroid.bat %FLAVOR% para gerar o build.
    goto :fail
)

REM Verifica se ha um dispositivo conectado
set "DEVICE="
for /f "skip=1 tokens=1,2" %%a in ('"%ADB%" devices') do if "%%b"=="device" set "DEVICE=%%a"
if not defined DEVICE (
    echo [ERRO] Nenhum dispositivo Android conectado/autorizado.
    echo Conecte o celular via USB, ative a depuracao USB e aceite a permissao na tela.
    goto :fail
)
echo Dispositivo: %DEVICE%

REM Preferir o script de instalacao gerado pela Unreal (trata APK + OBB)
for %%f in ("%BUILD_DIR%\Install_*.bat") do (
    echo Usando instalador da Unreal: %%~nxf
    pushd "%BUILD_DIR%"
    call "%%~nxf"
    popd
    goto :launch
)

REM Fallback: instala o APK direto
set "APK="
for %%f in ("%BUILD_DIR%\*.apk") do set "APK=%%f"
if not defined APK (
    echo [ERRO] Nenhum .apk encontrado em %BUILD_DIR%
    goto :fail
)
echo Instalando %APK% ...
"%ADB%" install -r "%APK%" || goto :fail

REM Copia OBB se existir (quando os dados nao estao dentro do APK)
for %%f in ("%BUILD_DIR%\*.obb") do (
    echo Enviando OBB %%~nxf ...
    "%ADB%" shell mkdir -p /sdcard/Android/obb/%PACKAGE%
    "%ADB%" push "%%f" /sdcard/Android/obb/%PACKAGE%/ || goto :fail
)

:launch
echo Abrindo o jogo...
"%ADB%" shell monkey -p %PACKAGE% -c android.intent.category.LAUNCHER 1 >nul 2>&1
echo Concluido.
endlocal
pause
exit /b 0

:fail
endlocal
pause
exit /b 1
