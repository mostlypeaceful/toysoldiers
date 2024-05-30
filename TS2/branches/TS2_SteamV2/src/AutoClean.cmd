@call "%VS90COMNTOOLS%vsvars32.bat"

@devenv /clean "game_internal|Win32" GameApp.sln
@devenv /clean "game_release|Win32" GameApp.sln
@devenv /clean "game_internal|Xbox 360" GameApp.sln
@devenv /clean "game_profile|Xbox 360" GameApp.sln
@devenv /clean "game_release|Xbox 360" GameApp.sln
