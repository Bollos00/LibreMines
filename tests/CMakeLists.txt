#############################################################################
# LibreMines Test Suite                                                     #
# Copyright (C) 2020-2025  Bruno Bollos Correa                              #
# This program is free software: you can redistribute it and/or modify      #
# it under the terms of the GNU General Public License as published by      #
# the Free Software Foundation, either version 3 of the License, or         #
# (at your option) any later version.                                       #
#                                                                           #
# This program is distributed in the hope that it will be useful,           #
# but WITHOUT ANY WARRANTY; without even the implied warranty of            #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
# GNU General Public License for more details.                              #
#                                                                           #
# You should have received a copy of the GNU General Public License         #
# along with this program.  If not, see <http://www.gnu.org/licenses/>.     #
#############################################################################

# Add Qt Test component
if(USE_QT6)
    find_package(Qt6 REQUIRED COMPONENTS Test)
else()
    find_package(Qt5 REQUIRED COMPONENTS Test)
endif()

# Enable testing
enable_testing()

# Include source directory for headers
include_directories(${PROJECT_SOURCE_DIR}/src)

# Create a static library with all source files (excluding main.cpp)
# This allows tests to link against the core functionality
set(LIBREMINES_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/extrathemes.cpp
    ${PROJECT_SOURCE_DIR}/src/facesreaction.cpp
    ${PROJECT_SOURCE_DIR}/src/gameboardrandomgenerator.cpp
    ${PROJECT_SOURCE_DIR}/src/keyboardcontroller.cpp
    ${PROJECT_SOURCE_DIR}/src/libreminesapptheme.cpp
    ${PROJECT_SOURCE_DIR}/src/libreminesgameengine.cpp
    ${PROJECT_SOURCE_DIR}/src/libreminespreferencesdialog.cpp
    ${PROJECT_SOURCE_DIR}/src/libreminespreferencessaver.cpp
    ${PROJECT_SOURCE_DIR}/src/libreminesscore.cpp
    ${PROJECT_SOURCE_DIR}/src/libreminesscoresdialog.cpp
    ${PROJECT_SOURCE_DIR}/src/libreminessolver.cpp
    ${PROJECT_SOURCE_DIR}/src/libreminesviewscoresdialog.cpp
    ${PROJECT_SOURCE_DIR}/src/minefieldtheme.cpp
    ${PROJECT_SOURCE_DIR}/src/qkeyinput.cpp
    ${PROJECT_SOURCE_DIR}/src/qlabel_adapted.cpp
    ${PROJECT_SOURCE_DIR}/src/soundeffects.cpp
)

# Create core library for testing
add_library(libremines_core STATIC ${LIBREMINES_CORE_SOURCES})

# Link Qt libraries to core library
if(USE_QT6)
    target_link_libraries(libremines_core 
        Qt6::Core 
        Qt6::Widgets 
        Qt6::Svg 
        Qt6::Multimedia
    )
else()
    target_link_libraries(libremines_core 
        Qt5::Core 
        Qt5::Widgets 
        Qt5::Svg 
        Qt5::Multimedia
    )
endif()

# Set up MOC for core library
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Function to create a test executable
function(add_libremines_test test_name test_source)
    add_executable(${test_name} ${test_source})
    
    # Link to core library and Qt Test
    if(USE_QT6)
        target_link_libraries(${test_name} 
            libremines_core 
            Qt6::Test 
            Qt6::Core 
            Qt6::Widgets
        )
    else()
        target_link_libraries(${test_name} 
            libremines_core 
            Qt5::Test 
            Qt5::Core 
            Qt5::Widgets
        )
    endif()
    
    # Add test to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Set test properties
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT 30
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

# Add unit tests
add_subdirectory(unit)
