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

## C++ Coding Best Practices

Follow these C++ coding standards and patterns that are established in the LibreMines codebase:

### File Structure and Headers
- **ALWAYS include the GPL license header** at the top of every new C++ file:
  ```cpp
  /*****************************************************************************
   * LibreMines                                                                *
   * Copyright (C) 2020-2025  Bruno Bollos Correa                              *
   *                                                                           *
   * This program is free software: you can redistribute it and/or modify      *
   * it under the terms of the GNU General Public License as published by      *
   * the Free Software Foundation, either version 3 of the License, or         *
   * (at your option) any later version.                                       *
   *                                                                           *
   * This program is distributed in the hope that it will be useful,           *
   * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
   * GNU General Public License for more details.                              *
   *                                                                           *
   * You should have received a copy of the GNU General Public License         *
   * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
   *****************************************************************************/
  ```
- **Use traditional include guards** (not `#pragma once`):
  ```cpp
  #ifndef CLASSNAME_H
  #define CLASSNAME_H
  // ... content ...
  #endif // CLASSNAME_H
  ```

### Naming Conventions
- **Classes**: PascalCase (e.g., `LibreMinesGui`, `MinefieldTheme`, `SoundEffects`)
- **Methods**: 
  - camelCase for getters and general methods (e.g., `getPixmapFromCellValue`)
  - Prefix `v` for void methods (e.g., `vNewGame`, `vSetMinefieldTheme`)
  - Qt signal/slot convention: `SIGNAL_name`, `SLOT_name`
- **Variables**:
  - Instance variables: hungarian notation style with type prefixes (`i` for int, `b` for bool, `n` for count)
  - Examples: `iX`, `iY`, `bFirst`, `nMines`, `cellLength`
  - Member pointers: `pm` prefix for pixmaps, descriptive names otherwise
- **Enums**: Use scoped enums (`enum class`) with PascalCase names and values
  ```cpp
  enum class CellValue: qint8 {
      MINE = -1,
      ZERO = 0,
      ONE = 1
  };
  ```

### Memory Management and Qt Patterns
- **Use Qt smart pointers** for automatic memory management:
  ```cpp
  QScopedPointer<QPixmap> pmMine;
  QPointer<LibreMinesGameEngine> gameEngine;
  ```
- **Follow Qt parent-child ownership** for GUI objects:
  ```cpp
  LibreMinesPreferencesDialog* preferences = new LibreMinesPreferencesDialog(this);
  ```
- **Use RAII principles** - initialize in constructors, cleanup automatic via destructors
- **Prefer Qt containers** over STL when working with Qt objects (`QString`, `QDir`, `QFile`)

### Class Design Patterns
- **Use nested classes** for tightly coupled helper classes:
  ```cpp
  class LibreMinesGui : public QMainWindow {
      class CellGui {
          // Helper class implementation
      };
  };
  ```
- **Initialize members in constructor initialization lists**:
  ```cpp
  LibreMinesGui::LibreMinesGui(QWidget *parent) :
      QMainWindow(parent),
      gameEngine(),
      iLimitHeightField(0),
      cellLength(0)
  {
  }
  ```
- **Use default member initialization** in headers for simple types:
  ```cpp
  class Example {
      int value = 0;
      bool flag = false;
  };
  ```

### Modern C++ Features (C++11+)
- **Use scoped enums** (`enum class`) instead of traditional enums
- **Use range-based for loops** when appropriate:
  ```cpp
  for(CellGameEngine& cell : row) {
      cell.value = CellValue::ZERO;
  }
  ```
- **Use lambda expressions** for Qt signal/slot connections:
  ```cpp
  connect(object, &Class::signal, [this](bool value) {
      this->handleSignal(value);
  });
  ```
- **Use auto** sparingly, prefer explicit types for clarity in Qt code

### Qt-Specific Conventions
- **Always use Q_OBJECT macro** in classes that inherit from QObject
- **Connect signals and slots** using new syntax when possible:
  ```cpp
  connect(sender, &SenderClass::signalName, receiver, &ReceiverClass::slotName);
  ```
- **Use Qt's debug system** instead of std::cout:
  ```cpp
  qDebug() << "Debug message with" << variable;
  ```
- **Check Qt object validity** before use in long-running operations
- **Use QStandardPaths** for platform-appropriate data directories

### Error Handling and Validation
- **Check file operations** before proceeding:
  ```cpp
  if (!file.open(QIODevice::ReadOnly)) {
      qDebug() << "Failed to open file:" << file.fileName();
      return;
  }
  ```
- **Validate parameters** in public methods, especially for game engine operations
- **Use Qt's built-in validation** for GUI input (QValidator classes)

### Code Organization
- **Group related functionality** in logical sections within classes
- **Use consistent indentation** (4 spaces, no tabs)
- **Keep line length reasonable** (prefer under 100 characters)
- **Separate concerns**: UI in GUI classes, logic in GameEngine, themes in Theme classes
- **Use forward declarations** in headers when possible to reduce compilation dependencies

### Resource Management
- **Use Qt resource system** for embedded files (themes, sounds, translations)
- **Load resources efficiently** - cache pixmaps, reuse objects
- **Clean up temporary files** and handle platform differences properly
- **Test resource loading** with both Qt5 and Qt6 compatibility

## Code Documentation Standards

LibreMines follows comprehensive JavaDoc/Doxygen-style documentation practices for all C++ header files. **ALWAYS** maintain these documentation standards when adding or modifying code.

### Documentation Style
- **Use JavaDoc/Doxygen format** with `@` tags consistently throughout
- **Document ALL public and protected** classes, methods, enums, and member variables
- **Follow established patterns** - examine existing header files for reference examples
- **Maintain professional quality** - documentation should be clear, accurate, and helpful

### File-Level Documentation
**REQUIRED** for every header file:
```cpp
/**
 * @file filename.h
 * @brief Concise description of file purpose
 * @author Bruno Bollos Correa
 * @date YYYY-MM-DD
 * 
 * Detailed description explaining the file's role in the application
 * architecture, its main components, and how it fits into the larger
 * system. Include key concepts and relationships to other components.
 */
```

### Historical Accuracy Requirements
- **@date field**: Use actual file creation date from Git history in ISO 8601 format (YYYY-MM-DD)
  - **NEVER use generic ranges** like "2020-2025"
  - **Research Git history**: `git log --follow --format=%ad --date=short -- filename.h | tail -1`
- **@since field**: Use actual version when class/feature was first introduced
  - **Research release tags**: Check Git tags and release history
  - Examples: v1.0, v1.1.0, v1.2.0, v1.2.2, v1.4.0, v1.5.0, v1.9.0

### Class Documentation
**REQUIRED** for every public/protected class:
```cpp
/**
 * @brief One-line summary of class purpose
 * 
 * Detailed description of the class responsibilities, key features,
 * and how it interacts with other components. Include:
 * - Primary responsibilities and role in application
 * - Key relationships with other classes
 * - Usage patterns and important behaviors
 * - Any architectural significance
 * 
 * @since vX.Y.Z
 */
class ClassName {
```

### Method Documentation
**REQUIRED** for all public/protected methods:
```cpp
/**
 * @brief Concise description of what the method does
 * 
 * Optional detailed description for complex methods including
 * behavior notes, side effects, or usage guidelines.
 * 
 * @param paramName Description of parameter purpose and constraints
 * @param anotherParam Description with type information if needed
 * @return Description of return value and possible values/states
 */
ReturnType methodName(Type paramName, Type anotherParam);
```

### Qt Signal/Slot Documentation
**REQUIRED** for all signals and slots:
```cpp
/**
 * @brief Description of when signal is emitted or slot is called
 * 
 * @param param Specific meaning of parameter in this signal context
 */
void SIGNAL_signalName(Type param);

/**
 * @brief Description of slot's action and response
 * 
 * @param param How this parameter affects the slot's behavior
 */
void SLOT_slotName(Type param);
```

### Enumeration Documentation
**REQUIRED** with inline comments for each value:
```cpp
/**
 * @brief Purpose and usage of this enumeration
 * 
 * Detailed description of when and how this enum is used,
 * including any important behavioral notes.
 * 
 * @since vX.Y.Z
 */
enum class EnumName {
    VALUE_ONE,      ///< Description of what this value represents
    VALUE_TWO,      ///< Description with behavioral context
    VALUE_THREE     ///< Description including usage notes
};
```

### Member Variable Documentation
**REQUIRED** for public/protected members:
```cpp
class ClassName {
public:
    int memberVariable;    ///< Description of variable purpose and usage
    bool flagMember;       ///< Description of what states this flag represents
};
```

### Cross-References and Links
- **Use @see tags** to link related classes, methods, or concepts:
  ```cpp
  * @see RelatedClass
  * @see RelatedClass::relatedMethod()
  * @see EnumName for possible values
  ```
- **Reference Qt documentation** when using Qt-specific patterns
- **Link to architectural components** that interact with the documented code

### Documentation Validation
**ALWAYS verify documentation accuracy:**
1. **Build successfully** - ensure documentation doesn't break compilation
2. **Check historical accuracy** - verify dates and versions against Git history
3. **Review completeness** - ensure all public/protected members are documented
4. **Test cross-references** - verify @see links point to existing components
5. **Validate examples** - ensure any code examples compile and work correctly

### Common Patterns in LibreMines
- **Game engine classes**: Document game logic, state management, and signal emissions
- **GUI classes**: Document user interaction, widget management, and Qt parent-child relationships
- **Theme classes**: Document resource loading, pixmap management, and Qt resource system usage
- **Utility classes**: Document helper functions, data processing, and configuration management
- **Dialog classes**: Document user workflow, settings persistence, and validation logic

**Remember**: Good documentation is essential for maintainability and contributor onboarding. Treat documentation as seriously as the code itself.