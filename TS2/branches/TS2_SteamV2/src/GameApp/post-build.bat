@echo off
setlocal

set DST=%1

copy /y "%SigEngine%\Src\External\Steamworks SDK 1.25\redistributable_bin\steam_api.dll" %DST%
copy /y "%SigEngine%\Src\External\Theora\lib\*.dll" %DST%

endlocal