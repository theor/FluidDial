# FindSDL2.cmake - Finds SDL2 and creates the SDL2::SDL2 imported target.
#
# Strategy:
#   1. Try find_package(SDL2 CONFIG) for vcpkg / system installs that provide
#      an SDL2Config.cmake.
#   2. Fall back to searching well-known MSYS2/MinGW paths on Windows.
#
# Result variables (after success):
#   SDL2_FOUND        - TRUE if SDL2 was found
#   SDL2_INCLUDE_DIRS - Include directories
#   SDL2_LIBRARIES    - Libraries to link
#
# Imported targets:
#   SDL2::SDL2        - The SDL2 library

# Guard against being included multiple times
if(TARGET SDL2::SDL2)
    return()
endif()

# --- Attempt 1: CONFIG mode (vcpkg, system package managers) ---
find_package(SDL2 CONFIG QUIET)
if(SDL2_FOUND AND TARGET SDL2::SDL2)
    return()
endif()

# --- Attempt 2: Manual search (MSYS2/MinGW on Windows) ---
set(_SDL2_SEARCH_PATHS
    "C:/msys64/mingw64"
    "C:/msys64/mingw32"
    "C:/msys2/mingw64"
    "C:/msys2/mingw32"
)

find_path(SDL2_INCLUDE_DIR
    NAMES SDL.h
    PATH_SUFFIXES SDL2 include/SDL2 include
    PATHS ${_SDL2_SEARCH_PATHS}
)

find_library(SDL2_LIBRARY
    NAMES SDL2 SDL2-2.0 libSDL2
    PATH_SUFFIXES lib lib64
    PATHS ${_SDL2_SEARCH_PATHS}
)

find_library(SDL2MAIN_LIBRARY
    NAMES SDL2main libSDL2main
    PATH_SUFFIXES lib lib64
    PATHS ${_SDL2_SEARCH_PATHS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR
)

if(SDL2_FOUND)
    set(SDL2_INCLUDE_DIRS "${SDL2_INCLUDE_DIR}")
    set(SDL2_LIBRARIES    "${SDL2_LIBRARY}")

    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2 PROPERTIES
            IMPORTED_LOCATION             "${SDL2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
        )
    endif()

    if(SDL2MAIN_LIBRARY AND NOT TARGET SDL2::SDL2main)
        add_library(SDL2::SDL2main UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2main PROPERTIES
            IMPORTED_LOCATION "${SDL2MAIN_LIBRARY}"
        )
    endif()
endif()
