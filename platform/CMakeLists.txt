add_library(shiba_platform STATIC)
add_library(shiba::platform ALIAS shiba_platform)

# Platform interface
target_include_directories(shiba_platform PUBLIC inc)

# OS-agnostic sources
target_sources(shiba_platform PRIVATE
        src/common.cpp
)

# Per-OS backend selection
if(WIN32)
    target_sources(shiba_platform PRIVATE
            src/windows/win32_surface.cpp
    )
endif ()

target_link_libraries(shiba_platform PUBLIC shiba::warns)
shiba_enable_ipo(shiba_platform)

set_target_properties(shiba_platform PROPERTIES FOLDER "engine")