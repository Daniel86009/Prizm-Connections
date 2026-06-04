@echo off
:: SDK PATH
set FXCGSDK=%~dp0..\PrizmSDK-win-0.6\

:: SDK bin folder
set PATH=%FXCGSDK%\bin;%PATH%

echo Checking for compiler...
if not exist "%FXCGSDK%\bin\make.exe" (
    echo [ERROR] Could not find make.exe at %FXCGSDK%\bin\make.exe
    echo Please edit this .bat file and set FXCGSDK to your actual SDK folder.
    pause
    exit /b
)

echo SDK Found! Starting Build...
echo ---------------------------------------
make %*
echo ---------------------------------------
echo Build finished.
pause