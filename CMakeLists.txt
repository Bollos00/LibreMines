#############################################################################
# LibreMines                                                                #
# Copyright (C) 2020-2023  Bruno Bollos Correa                              #                                                                         #
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

message(STATUS "Using CMake version ${CMAKE_VERSION}")
cmake_minimum_required(VERSION 3.1.0)
project(libremines
    VERSION "1.10.0"
    DESCRIPTION " A Free/Libre and Open Source Software Qt based Minesweeper game available for GNU/Linux, FreeBSD and Windows systems. "
    HOMEPAGE_URL "https://github.com/Bollos00/LibreMines"
    LANGUAGES "CXX"
    )

# Add the Qt dependencies according to the option `USE_QT6`
if(USE_QT6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt6 REQUIRED COMPONENTS Core)
    find_package(Qt6 REQUIRED COMPONENTS Svg)
    find_package(Qt6 REQUIRED COMPONENTS Multimedia)
    # If the option UPDATE_TRANSLATIONS is passed, the Qt5LinguistTools is needed
    #  to generate the QM Files
    if(UPDATE_TRANSLATIONS)
        find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
    endif()
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
    find_package(Qt5 REQUIRED COMPONENTS Core)
    find_package(Qt5 REQUIRED COMPONENTS Svg)
    find_package(Qt5 REQUIRED COMPONENTS Multimedia)
    if(UPDATE_TRANSLATIONS)
        find_package(Qt5 REQUIRED COMPONENTS LinguistTools)
    endif()
endif()

# Save the variables of the project (see file `include/cmake/libreminesconfig.h.in`) on
#  the file ${PROJECT_BINARY_DIR}/libreminesconfig.h
configure_file(${PROJECT_SOURCE_DIR}/include/libreminesconfig.h.in ${PROJECT_BINARY_DIR}/libreminesconfig.h)

# Create the translation files (QM File Format) from the ts files
if(UPDATE_TRANSLATIONS)
    set(TS_FILES
        etc/translations/${PROJECT_NAME}_es.ts
        etc/translations/${PROJECT_NAME}_pt_BR.ts
        )
    # Put the QM Files on `etc/translations`
    set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/etc/translations)
    if(USE_QT6)
        qt6_add_translation(QM_FILES ${TS_FILES})
    else()
        qt5_add_translation(QM_FILES ${TS_FILES})
    endif()
endif()

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    message(STATUS "Using default build type Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
string(COMPARE EQUAL "${CMAKE_CXX_STANDARD}" "" no_cmake_cxx_standard_set)
if(no_cmake_cxx_standard_set)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    message(STATUS "Using default C++ standard ${CMAKE_CXX_STANDARD}")
else()
    message(STATUS "Using user specified C++ standard ${CMAKE_CXX_STANDARD}")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(${PROJECT_NAME} WIN32
    etc/translations/libremines_translations_qm.qrc
    src/common.cpp
    src/common.h
    src/main.cpp
    src/libreminesapptheme.cpp
    src/libreminesapptheme.h
    src/libreminesgameengine.cpp
    src/libreminesgameengine.h
    src/libreminesgui.cpp
    src/libreminesgui.h
    src/libreminespreferencesdialog.cpp
    src/libreminespreferencesdialog.h
    src/libreminespreferencesdialog.ui
    src/libreminespreferencessaver.cpp
    src/libreminespreferencessaver.h
    src/libreminesscore.cpp
    src/libreminesscore.h
    src/libreminesscoresdialog.cpp
    src/libreminesscoresdialog.h
    src/libreminesviewscoresdialog.cpp
    src/libreminesviewscoresdialog.h
    src/minefieldextratheme.cpp
    src/minefieldextratheme.h
    src/minefieldtheme.cpp
    src/minefieldtheme.h
    src/qkeyinput.cpp
    src/qkeyinput.h
    src/qlabel_adapted.cpp
    src/qlabel_adapted.h
    src/qpushbutton_adapted.cpp
    src/qpushbutton_adapted.h
    src/soundeffects.cpp
    src/soundeffects.h
    share/application_stylesheets/qdarkstyle/dark/qdarkstyle_dark.qrc
    share/application_stylesheets/qdarkstyle/light/qdarkstyle_light.qrc
    share/application_stylesheets/qss/qss.qrc
    share/facesreaction.qrc
    share/icons.qrc
    share/icons.rc
    share/minefield_themes.qrc
    share/sound_effects.qrc
    )

include_directories(src)
include_directories(include)
include_directories(${PROJECT_BINARY_DIR})


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DEPRECATED_WARNINGS)

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x060000)
# disables all the APIs deprecated before Qt 6.0.0


# Link the Qt dependencies according to the option `USE_QT6`
if(USE_QT6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Svg)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Multimedia)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Svg)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Multimedia)
endif()

# Set the install settings for Unix environments
if(UNIX)
    # create install target
    include(GNUInstallDirs)

    # Install the executable
    install(
        TARGETS
        ${PROJECT_NAME}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

    # Install the desktop entry and the icon
    install(FILES share/desktop_entry/libremines.desktop DESTINATION share/applications)
    install(FILES share/icons/libremines.svg DESTINATION share/icons/hicolor/scalable/apps)

    # Install the extra minefield themes on
    #  ${CMAKE_INSTALL_PREFIX}/share/libremines/minefield_extra_themes
    if(INSTALL_EXTRA_MINEFIELD_THEMES)
        install(
            DIRECTORY share/minefield_themes/alternative_light
            DESTINATION share/libremines/minefield_extra_themes
            )
        install(
            DIRECTORY share/minefield_themes/alternative_dark
            DESTINATION share/libremines/minefield_extra_themes
            )
        install(
            DIRECTORY share/minefield_themes/flowerfield
            DESTINATION share/libremines/minefield_extra_themes
            )

    endif()

elseif(WIN32)

endif()
