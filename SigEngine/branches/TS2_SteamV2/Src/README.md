## Video Encoding/Decoding

### Ogg Theora

This project includes functionality for video encoding/decoding using the Theora codec.

- **Theora Codec**: An open video codec for encoding and decoding video streams.
- **License**: Theora is distributed under a BSD-style license. See the `LICENSE` file for more details.
- **More Information**: [Theora Codec](https://www.theora.org/)

## External Dependencies

### Microsoft ATL and MFC Libraries

This project requires the Microsoft ATL (Active Template Library) and MFC (Microsoft Foundation Class) libraries for building and running certain components.

- **Note**: The `atlmfc` folder has been removed due to proprietary licensing restrictions. Users will need a valid Visual Studio license to access these libraries.
- **Installation Instructions**: To install ATL and MFC, follow these steps:
  1. Open Visual Studio Installer.
  2. Select the workload that includes ATL and MFC support (e.g., "Desktop development with C++").
  3. Ensure that "C++ ATL for latest v142 build tools" and "C++ MFC for latest v142 build tools" are selected.
  4. Install the selected components.

Refer to the [Visual Studio Documentation](https://docs.microsoft.com/en-us/visualstudio/) for more details on installing and configuring these libraries.

### Removed Xbox 360 Libraries

Xbox 360-specific library folders have been removed:

- **game_xbox360_debug**
- **game_xbox360_internal**
- **game_xbox360_release**

### Notes

- Ensure that all paths and settings are correctly configured for your development environment.
- Refer to the specific documentation for any third-party dependencies referenced in these files.
