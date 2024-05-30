@call "%VS90COMNTOOLS%vsvars32.bat"


@devenv GameApp.sln /build "game_internal|Win32"
@if errorlevel 1 goto end_error

@devenv GameApp.sln /build "game_release|Win32"
@if errorlevel 1 goto end_error

@devenv GameApp.sln /build "game_internal|Xbox 360"
@if errorlevel 1 goto end_error

@devenv GameApp.sln /build "game_profile|Xbox 360"
@if errorlevel 1 goto end_error

@devenv GameApp.sln /build "game_playtest|Xbox 360"
@if errorlevel 1 goto end_error

@devenv GameApp.sln /build "game_release|Xbox 360"
@if errorlevel 1 goto end_error

goto end_success

:end_error
@echo Error building game projects.

:end_success
