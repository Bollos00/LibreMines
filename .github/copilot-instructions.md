# LibreMines - Qt6 Minesweeper Game

LibreMines is a Free/Libre and Open Source Qt6-based Minesweeper clone available for GNU/Linux, FreeBSD, macOS and Windows systems. The application is written in C++ and uses CMake as its build system.

**ALWAYS reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.**

## Working Effectively

### Bootstrap and Build Process
- Install dependencies for Ubuntu 24.04+ (Qt6 is default and required):
  ```
  sudo apt-get update
  sudo apt-get install build-essential qt6-base-dev cmake libqt6svg6-dev qt6-multimedia-dev libgl1-mesa-dev
  ```
- Configure the build system:
  ```
  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
  ```
  - Takes approximately 2 seconds. NEVER CANCEL. Set timeout to 60+ seconds.
- Build the application:
  ```
  make -j$(nproc)
  ```
  - Takes approximately 22 seconds on modern systems. NEVER CANCEL. Set timeout to 180+ seconds.
- Install the application (optional):
  ```
  make install
  ```
  - Takes approximately 0.1 seconds. NEVER CANCEL. Set timeout to 30+ seconds.

### Alternative Qt5 Build (Limited Support)
- For systems with Qt5 only, use: `cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DUSE_QT6="NO" ..`
- Note: Qt5 packages may not be available on modern Ubuntu versions (24.04+)

## Validation

### Essential Testing Commands
- **ALWAYS test the executable after building:**
  ```
  QT_QPA_PLATFORM=offscreen ./libremines --version
  QT_QPA_PLATFORM=offscreen ./libremines --help
  ```
- **Manual functionality validation (CRITICAL):**
  ```
  timeout 10s bash -c "QT_QPA_PLATFORM=offscreen ./libremines --width 800 --heigth 600"
  ```
  - Should run for 10 seconds without crashing (exit code 124 is expected from timeout)
  - Audio warnings about missing PulseAudio are normal in headless environments
  - This confirms the GUI loads successfully in offscreen mode

### Complete Validation Scenarios
- **ALWAYS run through these validation steps after making changes:**
  1. Build the application successfully
  2. Test `--version` and `--help` commands work
  3. Launch the application in offscreen mode to verify GUI initialization
  4. Check that the executable is created in the build directory
  5. Verify installation works if relevant to your changes

### No Automated Testing Available
- **CRITICAL**: There are NO unit tests or automated test suites in this repository
- **CRITICAL**: There are NO linting or code formatting tools configured
- All validation must be done manually using the commands above
- The GitHub Actions workflows (.github/workflows/) test building on Ubuntu, Arch Linux, and Windows

## Common Tasks

### Build Targets and Timing
- `make` or `make all` - Build the entire application (~22 seconds, NEVER CANCEL, timeout 180+ seconds)
- `make clean` - Clean build artifacts (~1 second)
- `make install` - Install to system (~0.1 seconds, timeout 30+ seconds) 
- `make help` - Show available targets

### File Locations and Navigation
- **Main executable source**: `src/main.cpp`
- **Primary GUI implementation**: `src/libreminesgui.cpp` and `src/libreminesgui.h`
- **Game engine logic**: `src/libreminesgameengine.cpp` and `src/libreminesgameengine.h`
- **Build configuration**: `CMakeLists.txt` (root directory)
- **Application themes and resources**: `share/` directory
- **Translation files**: `etc/translations/`
- **Screenshots and documentation**: `share/Screenshots/`

### Key Project Components
The application consists of several main components:
- **LibreMinesGui**: Main user interface and window management
- **LibreMinesGameEngine**: Core minesweeper game logic and state
- **MinefieldTheme**: Theme and visual appearance management
- **LibreMinesPreferences**: Settings and configuration management
- **KeyboardController**: Keyboard input handling for accessibility
- **SoundEffects**: Audio feedback system

### Dependencies and System Requirements
- **Qt6 Core** >= 6.0 (QtCore6)
- **Qt6 Widgets** >= 6.0 (QtWidgets6) 
- **Qt6 SVG** >= 6.0 (QtSvg6)
- **Qt6 Multimedia** >= 6.0 (QtMultimedia6)
- **CMake** >= 3.1.0
- **C++11 compatible compiler** (GCC 13.3+ on Ubuntu 24.04)
- **OpenGL development libraries** (libgl1-mesa-dev)

### Development Workflow
1. **ALWAYS build and test after making changes**
2. **Use the validation commands above to ensure functionality**
3. **For GUI changes, test with offscreen mode to verify the interface loads**
4. **Check both Qt6 and Qt5 compatibility if modifying CMakeLists.txt**
5. **Verify resource files (themes, icons, sounds) load correctly after changes**

### Troubleshooting
- **Build failures**: Check Qt6 development packages are installed correctly
- **Runtime issues**: Use `QT_QPA_PLATFORM=offscreen` for headless testing
- **Missing dependencies**: Refer to the dependency installation commands above
- **CMake configuration issues**: Ensure CMake >= 3.1.0 and all Qt6 components are available
- **Linking errors**: Verify all required Qt6 modules are properly linked in CMakeLists.txt

### CI/CD Integration
- The repository includes GitHub Actions workflows for automated building
- **Ubuntu workflow**: Tests on Ubuntu 20.04 (Qt5) and 22.04+ (Qt6)
- **Windows workflow**: Tests Windows builds with Qt6
- **Arch Linux workflow**: Tests on Arch Linux with latest packages
- All workflows follow the same build pattern: dependencies → cmake → make

**REMEMBER**: Always validate your changes using the manual testing procedures above since there are no automated tests to catch regressions.