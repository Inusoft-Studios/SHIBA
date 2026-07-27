add_library(shiba_llr STATIC)
add_library(shiba::llr ALIAS shiba_llr)

# Platform interface
target_include_directories(shiba_llr PUBLIC inc)

# OS-agnostic sources
target_sources(shiba_llr PRIVATE
        src/llr.cpp
)

# Per-OS backend selection
if(WIN32)
    target_sources(shiba_llr PRIVATE
            src/windows/win32_surface.cpp
    )
endif ()

target_link_libraries(shiba_llr PUBLIC
        shiba::warns
        shiba::platform
        shiba::core
        # TODO: shiba::resources
)
shiba_enable_ipo(shiba_llr)

# TODO: Setup per-backend module linking
set(SHIBA_GFX_BACKEND "Vulkan" CACHE STRING "Graphics backend")
set_property(CACHE SHIBA_GFX_BACKEND PROPERTY STRINGS Vulkan D3D12 OpenGL Null)

if(SHIBA_GFX_BACKEND STREQUAL "Vulkan")
    find_package(Vulkan 1.4.350 REQUIRED)
    target_sources(shiba_llr PRIVATE
            src/vulkan/gfx_vk.cpp
            src/vulkan/vk_mem_alloc.cpp
    )
    target_link_libraries(shiba_llr PRIVATE shiba::vma Vulkan::Vulkan)
    target_compile_definitions(shiba_llr PRIVATE SHIBA_GFX_VULKAN=1)
endif()

set_target_properties(shiba_llr PROPERTIES FOLDER "engine")