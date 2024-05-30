@echo off
setlocal

call "%VS110COMNTOOLS%vsvars32.bat"

p4 edit "%SigCurrentProject%\src\GameApp\WWise_IDs.h"
set WWISEROOT=%ProgramFiles(x86)%\Audiokinetic\Wwise v2011.1.2 build 3891
"%WWISEROOT%\Authoring\Win32\Release\bin\wwisecli.exe" "%SigCurrentProject%\res\Audio\~wwise\~wwise.wproj" -GenerateSoundBanks -Language English(US) -Platform Windows -HeaderFilePath "%SigCurrentProject%\src\GameApp"

devenv GameApp_VS2012.sln /build "game_internal|Win32"
if errorlevel 1 goto end_error

devenv GameApp_VS2012.sln /build "game_release|Win32"
if errorlevel 1 goto end_error

goto end_success

:end_error
echo Error building game projects.

:end_success
endlocal
