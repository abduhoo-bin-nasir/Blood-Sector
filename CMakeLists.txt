cmake_minimum_required(VERSION 3.15)
project(Blood-Sector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ─── Find or Fetch raylib (Fully Relative & Portable) ────────────────────────
find_package(raylib QUIET)

if(NOT raylib_FOUND)
    # Check common system paths relative to standard installation drives
    find_library(RAYLIB_LIB NAMES raylib libraylib
        PATHS
            "C:/raylib/lib"
            "C:/raylib/raylib/src"
            "$ENV{USERPROFILE}/raylib/lib"
    )
    find_path(RAYLIB_INCLUDE NAMES raylib.h
        PATHS
            "C:/raylib/include"
            "C:/raylib/raylib/src"
            "$ENV{USERPROFILE}/raylib/include"
    )

    if(RAYLIB_LIB AND RAYLIB_INCLUDE)
        # If found on C:/ drive locally, use it
        add_library(raylib STATIC IMPORTED)
        set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION "${RAYLIB_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE}"
        )
        message(STATUS "Found raylib locally at: ${RAYLIB_LIB}")
    else()
        # ULTIMATE FALLBACK: Download and compile raylib automatically for the user
        message(STATUS "Raylib not found locally. Fetching from GitHub...")
        FetchContent_Declare(
            raylib
            URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
        )
        # Force Raylib to build statically and skip unnecessary examples
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(WITH_PIC ON CACHE BOOL "" FORCE)
        
        FetchContent_MakeAvailable(raylib)
    endif()
endif()

# ─── Sources ──────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Entity.cpp
    src/Map.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Renderer.cpp
    src/Game.cpp
    src/LevelManager.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} raylib)

# ─── Static Linking & Platform Specifics ──────────────────────────────────────
if(WIN32)
    # Force GCC/MinGW to bake standard C++ DLLs directly into the executable
    target_link_options(${PROJECT_NAME} PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
    
    # Required Windows system libraries for Raylib
    target_link_libraries(${PROJECT_NAME} winmm opengl32 gdi32)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} m pthread dl)
endif()