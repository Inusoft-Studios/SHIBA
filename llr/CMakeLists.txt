add_library(shiba_llr STATIC)
add_library(shiba::llr ALIAS shiba_llr)

# Platform interface
target_include_directories(shiba_llr PUBLIC inc)

# OS-agnostic sources
target_sources(shiba_llr PRIVATE
        src/llr.cpp
)

# TODO: Per-OS backend selection

target_link_libraries(shiba_llr PUBLIC
        shiba::warns
        shiba::platform
        shiba::core
        # TODO: shiba::resources
)
shiba_enable_ipo(shiba_llr)
set_target_properties(shiba_llr PROPERTIES FOLDER "engine")