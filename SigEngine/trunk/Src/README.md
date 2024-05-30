## Build Configuration Files

### MSBuild Property Files (`.props`)

These files define reusable properties for the project and are used in the build process:

- **enginelib_game_pcdx9_debug.props**: Configuration for DirectX 9 debug build.
- **enginelib_game_pcdx9_release.props**: Configuration for DirectX 9 release build.
- **enginelib_game_pcdx9_internal.props**: Internal settings for DirectX 9 builds.
- **enginelib_game_pcdx9_playtest.props**: Playtest configuration for DirectX 9 builds.
- **enginelib_game_pcdx9_profile.props**: Profile settings for DirectX 9 builds.

### Visual Studio Property Sheets (`.vsprops`)

These files are used for older versions of Visual Studio to define shared project settings:

- **enginelib_game_pcdx9_debug.vsprops**: Configuration for DirectX 9 debug build.
- **enginelib_game_pcdx9_release.vsprops**: Configuration for DirectX 9 release build.
- **enginelib_game_pcdx9_internal.vsprops**: Internal settings for DirectX 9 builds.
- **enginelib_game_pcdx9_playtest.vsprops**: Playtest configuration for DirectX 9 builds.
- **enginelib_game_pcdx9_profile.vsprops**: Profile settings for DirectX 9 builds.

### Notes

- Ensure that all paths and settings are correctly configured for your development environment.
- Refer to the specific documentation for any third-party dependencies referenced in these files.
