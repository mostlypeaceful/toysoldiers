## Overview

# SigEngine - Toy Soldiers Complete

## Overview

This repository contains the base SigEngine code used for the Toy Soldiers Complete Steam Version (XBLA Cold War on Xbox 360-I think. Pretty sure. Almost positive. SO many branches). This codebase is originally from around 2008-2013 ish and is being released as-is for public download and modification.

Apologies, i couldn't find much useful documentation or HOW TOOs. There is a lot more to know than what is below, but CHEMDAWG <> GOLDEN PINEAPPLE recall for me and Max/Ryan/Matt were the smart ones. Keep in mind the engine was developed specifically for Toy Soldiers gameplay and the limitations inherent to developing for xbox360 XBLA...lots of sharing, faking, 2gb package, 512 ram 'ish....' Personally, i feel compelled to return to this type of restrictive development. I lost interest in this latest round of 100gb games. I'm one of them dinosaurs now.

These bits should get you started in a direction and I'll add from time to time:

## Tech

- **DirectX 9**
- **Windows**
- **C**
- **C++**
- **C#**
- **.NUT (Squirrel scripting language)**
- **XML**

## Installation and Setup

1. **Clone the Repository**

   ```sh
   git clone https://github.com/mostlypeaceful/toysoldiers.git
   cd toysoldiers

   ```

2. **Build the Project**

   Make sure you have the necessary dependencies and build tools installed. Obstacles and errors abound but you'll make it out alive and learn stuff.

## Contributing

1. **Fork the Repository**

   Click the "Fork" button at the top right of this page to create a copy of this repository on your GitHub account.

2. **Create a Branch**

   ```sh
   git checkout -b feature/YourFeatureName
   ```

3. **Commit Your Changes**

   ```sh
   git add .
   git commit -m 'Add some feature'
   ```

4. **Push to the Branch**

   ```sh
   git push origin feature/YourFeatureName
   ```

5. **Open a Pull Request**

   Navigate to the original repository on GitHub and open a pull request from your fork.

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Disclaimer

**USE AT YOUR OWN RISK.** This code is provided as-is, with no warranties or guarantees. Game code bases can be complex and may/does contain bugs, rabbit holes, or incomplete features. There is no plan or expectation for bug fixes or support from the original authors. It is highly recommended that you have solid problem solving and technical skills to work with this engine.

## Contact

For any queries, please open an issue on this repository or contact me directly through my GitHub profile or @toysoldiersgame on X. BEST IS X.

## Internal Tools and Libraries

1. **AssetGen**: Tool for generating game assets and building the game.
2. **AssetPlugins**: Plugins for handling various asset types.
3. **Atlas**: Manages texture atlases.
4. **AutoBuild**: Automates the build process.
5. **AutoTest**: Automates testing procedures.
6. **CheckIn**: Manages check-ins to version control.
7. **CleanBuildUI**: Cleans the build directory for UI components.
8. **CodePreproc**: Handles code preprocessing tasks.
9. **CrashDebug**: Tools for debugging crashes.
10. **CrashDumpTree**: Analyzes and organizes crash dump files.
11. **DataTableSchema**: Manages data table schemas.
12. **DesyncReader**: Reads and analyzes desynchronization logs.
13. **DownResRes**: Manages resource resolution downscaling.
14. **FileToByteArray**: Utility for converting files to byte arrays.
15. **FindAndFixMissingLocStrings**: Finds and fixes missing localization strings.
16. **GameLauncher**: Launches the game application.
17. **GameReplayer**: Replays game sessions for testing or debugging.
18. **GetLatest**: Fetches the latest version from version control.
19. **LocmlClean**: Cleans localization files.
20. **MaterialGen**: Generates material definitions and files.
21. **MaxPlugin**: Plugin for 3ds Max, for importing/exporting assets.
22. **MayaPlugin**: Plugin for Autodesk Maya, used for creating and managing 3D assets.
23. **P4Delta**: Custom Perforce tool for summarizing differences between local files and the repository.
24. **PlatformDebuggingTester**: Tools for debugging platform-specific issues.
25. **Prefinery Inviter**: Manages invites, possibly for beta testing or user feedback.
26. **ProjectSelector**: Tool for selecting and auto managing project environments.
27. **ResourceMover**: Manages the movement of resource files.
28. **RuidGen**: Generates unique resource identifiers.
29. **SigAI**: Tools for LOL "artificial intelligence" components.
30. **SigAnim**: Tools for animation systems.
31. **SigEd**: Terrain, World Building Editor tools for the SigEngine.
32. **SigEdPlugins**: Plugins for the SigEngine editor.
33. **SigFx**: Tools for particles.
34. **SigScript**: Scripting engine and related tools.
35. **SigShade**: Shading and rendering utilities.
36. **SigTile**: Tile-based map and level handling. Unclear where this was at here.
37. **SigUI**: User interface components.
38. **SyncCompare**: Synchronization and comparison utilities.
39. **UpdateXLAST**: Updates the XLAST tool, possibly for Xbox Live Arcade submissions.
40. **xbWatson**: Tools for integration with Xbox Watson, a crash reporting and diagnostics tool for Xbox.

## External Tools and Libraries

### Included Libraries

1. **bullet-2.66**

   - **Description**: Bullet is a physics engine that simulates collision detection, soft and rigid body dynamics. It is widely used in games, visual effects, and robotics simulations.
   - **License**: [Zlib license](http://opensource.org/licenses/Zlib).

2. **cml-1.0_2**

   - **Description**: Configurable Math Library (CML) is a C++ vector, matrix, and quaternion math library for games and graphics programming.
   - **License**: Typically permissive. Please check the specific version used.

3. **enet-1.3.0**

   - **Description**: ENet is a networking library designed for reliable UDP communication. It provides a simple, robust communication layer on top of UDP.
   - **License**: [MIT License](https://opensource.org/licenses/MIT).

4. **ENetCS**

   - **Description**: A C# wrapper for the ENet networking library, allowing its use in C# projects.
   - **License**: Same as ENet, [MIT License](https://opensource.org/licenses/MIT).

5. **rapidxml**

   - **Description**: RapidXML is a fast and simple C++ XML parser designed for performance and efficiency.
   - **License**: [Boost Software License](https://www.boost.org/users/license.html).

6. **squirrel**

   - **Description**: Squirrel is a high-level, dynamically typed programming language, designed as a lightweight scripting language.
   - **License**: [MIT License](https://opensource.org/licenses/MIT).

7. **tinyxml**

   - **Description**: TinyXML is a simple and small C++ XML parser that can be easily integrated into other applications.
   - **License**: [Zlib license](http://opensource.org/licenses/Zlib).

8. **Tweensy 0.2.2**

   - **Description**: A library for tweening (interpolating values) in animation, particularly useful in game development.
   - **License**: Verify the specific license of the version used.

9. **wxWidgets-2.8.7**

   - **Description**: wxWidgets is a C++ library that allows developers to create applications for multiple platforms with a single code base.
   - **License**: [wxWidgets License](https://opensource.org/licenses/wxWindows), similar to LGPL.

10. **zlib123**
    - **Description**: Zlib is a compression library used for data compression and decompression.
    - **License**: [Zlib License](http://opensource.org/licenses/Zlib).

### Deleted Libraries -dont need 360 SDK, others need to be replaced or licensed...audio for example relies on wwise and fmod

1. **Microsoft Xbox 360 SDK**

   - **Description**: Proprietary software development kit for Xbox 360 development.
   - **Status**: Deleted due to proprietary licensing.
   - **More Information**: [Microsoft Xbox 360 SDK](https://www.microsoft.com).

2. **fmod**

   - **Description**: FMOD is an audio engine for games and applications that provides sound effects and music playback.
   - **Status**: Deleted due to proprietary licensing.
   - **More Information**: [FMOD](https://www.fmod.com).

3. **Iggy**

   - **Description**: Iggy is a system for creating graphical user interfaces using Adobe Flash content.
   - **Status**: Deleted due to proprietary licensing.
   - **More Information**: [Iggy by RAD Game Tools](https://www.radgametools.com/iggy.htm).

4. **maya2010, maya2012, maya2013**

   - **Description**: Autodesk Maya is a 3D computer graphics application used for creating interactive 3D applications, including video games, animated film, TV series, or visual effects.
   - **Status**: Deleted due to proprietary licensing.
   - **More Information**: [Autodesk Maya](https://www.autodesk.com/products/maya/overview).

5. **wwise**
   - **Description**: Wwise is an audio middleware solution for creating dynamic sound environments in games.
   - **Status**: Deleted due to proprietary licensing.
   - **More Information**: [Wwise by Audiokinetic](https://www.audiokinetic.com/).

# file types

- **mshml** mesh files
- **sigml** sort of like prefabs--references of things to make a thing
- **animl** anims
- **sklml** skins
- **anipk** animation packs
- **sacml**
- **derml** material shaders

## name your texture this way:

- **name_d** color
- **name_n** normal
- **name_s** spec
- **name_g** gui
- **name_o** occlusion
- **name_i** self illumination
- **name_r** reflection (cube)
- **name_t** lookup gradient

**swizzling** a technique used to optimize memory usage by rearranging texture data. This method involves storing different texture maps in the Red, Green, and Blue channels of a single texture, reducing the overall memory footprint and improving performance.

the naming lets the engine know which compression to use, how to handle, etc.

you will need a RES DIRECTORY(check docs in sigeng) in your "TS2".....connect that to your SigEngine via Project Selector...
-maya 2013 is the art "editor" for the engine-you export animations, apply dermls, export meshml, skins, use layers for states....
-embedded shaders in mshml..ick
-vulkan? dx11?
-replace lockstep

Github bites for anything but small projects but also doesn't bite cause its easily accessible...anyway i dumped about 50gb of stuff (mostly worhtless 'cept content) so ill be adding content source as well to this in chunks...

<<<<< IF YOU'RE BASED >>>>>>> contact me directly me @toysoldiersgame on X and ill get you src content more and quicker.

i was gonna maybe try and pump AI agents into this thing or something...pretty sure im not smart enough. Maybe ill still try anyway. Would be cool..so many things to do with that...

E pluribus unum

PEACE FRENS
