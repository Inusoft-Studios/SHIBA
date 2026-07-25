cmake_minimum_required(VERSION 3.27)

set(SHIBA_PROJECT_VERSION_MAJOR 0)
set(SHIBA_PROJECT_VERSION_MINOR 0)
set(SHIBA_PROJECT_VERSION_PATCH 1)

# === Project ===
set(SHIBA_CXX_STANDARD_VERSION 17)
project(SHIBA
        VERSION "${SHIBA_PROJECT_VERSION_MAJOR}.${SHIBA_PROJECT_VERSION_MINOR}.${SHIBA_PROJECT_VERSION_PATCH}"
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD ${SHIBA_CXX_STANDARD_VERSION})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Don't accept in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are disabled. Use e.g. `cmake -S . -B build/{platform}`.")
endif()

get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# === CMake ===
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/.cmake")

option(SHIBA_WARNINGS_AS_ERRORS "Treat compiler warnings as errors"     ON)
option(SHIBA_ENABLE_SANITIZERS  "Enable address sanitizer + UBSan"      ON)

# Unified output dirs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler warnings interface to compile against
include(compiler_warnings)
add_library(shiba_warns INTERFACE)
add_library(shiba::warns ALIAS shiba_warns)
target_compile_features(shiba_warns INTERFACE cxx_std_${SHIBA_CXX_STANDARD_VERSION})
shiba_set_project_warnings(shiba_warns ${SHIBA_WARNINGS_AS_ERRORS})

# Address sanitizer + UBSan
include(sanitizers)

# IPO
include(ipo)

# === Layers ===
add_subdirectory(platform)
add_subdirectory(core)
add_subdirectory(resources)

# === Output ===
message(STATUS "")
message(STATUS "shiba ${PROJECT_VERSION} configured:")
message(STATUS "  Generator          : ${CMAKE_GENERATOR}")
message(STATUS "  Build type         : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler           : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Warnings as errors : ${SHIBA_WARNINGS_AS_ERRORS}")
message(STATUS "  Sanitizers         : ${SHIBA_ENABLE_SANITIZERS}")