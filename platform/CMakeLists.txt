add_library(shiba_platform STATIC)
add_library(shiba::platform ALIAS shiba_platform)

# Platform interface
target_include_directories(shiba_platform
        PUBLIC inc
        PRIVATE internal
)

# OS-agnostic sources
target_sources(shiba_platform PRIVATE
        src/common.cpp
        src/gfx.cpp
        src/timer.cpp
        src/version.cpp
)

# Per-OS backend selection
if(WIN32)
    target_sources(shiba_platform PRIVATE
            src/windows/win32_surface.cpp
            src/windows/win32_timer.cpp
    )
endif ()

target_link_libraries(shiba_platform PUBLIC shiba::warns)
shiba_enable_ipo(shiba_platform)

# Engine versioning within platform
target_compile_definitions(shiba_platform PRIVATE
        SHIBA_VERSION=${PROJECT_VERSION}
        SHIBA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        SHIBA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        SHIBA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# TODO: Setup per-backend module linking
set(SHIBA_GFX_BACKEND "Vulkan" CACHE STRING "Graphics backend")
set_property(CACHE SHIBA_GFX_BACKEND PROPERTY STRINGS Vulkan D3D12 OpenGL Null)

if(SHIBA_GFX_BACKEND STREQUAL "Vulkan")
    find_package(Vulkan 1.4.350 REQUIRED)
    target_sources(shiba_platform PRIVATE
            src/vulkan/gfx_vk.cpp
            src/vulkan/vk_mem_alloc.cpp
    )
    target_link_libraries(shiba_platform PRIVATE shiba::vma Vulkan::Vulkan)
    target_compile_definitions(shiba_platform PRIVATE SHIBA_GFX_VULKAN=1)
    message(STATUS "shiba: selected rendering api = vulkan")
endif()

set_target_properties(shiba_platform PROPERTIES FOLDER "engine")