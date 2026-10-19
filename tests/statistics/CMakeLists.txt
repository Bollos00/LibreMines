#############################################################################
# LibreMines - Optional solver statistics / benchmark tools                 #
# Copyright (C) 2020-2026  Bruno Bollos Correa                              #
# This program is free software: you can redistribute it and/or modify      #
# it under the terms of the GNU General Public License as published by      #
# the Free Software Foundation, either version 3 of the License, or         #
# (at your option) any later version.                                       #
#                                                                           #
# This program is distributed in the hope that it will be useful,           #
# but WITHOUT ANY WARRANTY; without even the implied warranty of            #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
# GNU General Public License for more details.                             #
#                                                                           #
# You should have received a copy of the GNU General Public License         #
# along with this program.  If not, see <http://www.gnu.org/licenses/>.     #
#############################################################################

# These executables are built only when -DBUILD_SOLVER_STATISTICS=ON is passed
# to CMake. They are meant to be run by hand and are intentionally NOT
# registered with CTest, so they never run as part of the normal test suite.

set(SOLVER_STATISTICS_COMMON_SOURCES
    deterministicboardgenerator.cpp
    deterministicboardgenerator.h
    solversolveharness.cpp
    solversolveharness.h
)

add_executable(solver_solvability_statistics
    solver_solvability_statistics.cpp
    ${SOLVER_STATISTICS_COMMON_SOURCES}
)

add_executable(solver_strategy_benchmark
    solver_strategy_benchmark.cpp
    ${SOLVER_STATISTICS_COMMON_SOURCES}
)

if(USE_QT6)
    set(SOLVER_STATISTICS_QT_LIBS Qt6::Core Qt6::Widgets)
else()
    set(SOLVER_STATISTICS_QT_LIBS Qt5::Core Qt5::Widgets)
endif()

foreach(target solver_solvability_statistics solver_strategy_benchmark)
    target_link_libraries(${target}
        libremines_core
        ${SOLVER_STATISTICS_QT_LIBS}
    )
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()
