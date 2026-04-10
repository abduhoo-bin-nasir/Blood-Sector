cmake_minimum_required(VERSION 3.15)
project(ShadowBreach)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Find raylib ──────────────────────────────────────────────────────────────
find_package(raylib QUIET)

if(NOT raylib_FOUND)
    # Fallback: common Windows install paths (adjust if yours differs)
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
        add_library(raylib STATIC IMPORTED)
        set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION "${RAYLIB_LIB}"
            INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE}"
        )
        message(STATUS "Found raylib manually at: ${RAYLIB_LIB}")
    else()
        message(FATAL_ERROR
            "\n[ERROR] raylib not found.\n"
            "Install raylib or set RAYLIB_LIB and RAYLIB_INCLUDE paths in CMakeLists.txt\n"
            "Download: https://github.com/raysan5/raylib/releases\n"
        )
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

# Platform-specific
if(WIN32)
    target_link_libraries(${PROJECT_NAME} winmm opengl32 gdi32)
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} m pthread dl)
endif()
