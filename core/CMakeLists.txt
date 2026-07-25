add_library(shiba_core STATIC)
add_library(shiba::core ALIAS shiba_core)

# Core headers
target_include_directories(shiba_core PUBLIC inc)

# Source files
target_sources(shiba_core PRIVATE
        src/core.cpp
)

target_link_libraries(shiba_core PUBLIC shiba::warns)
shiba_enable_ipo(shiba_core)

set_target_properties(shiba_core PROPERTIES FOLDER "engine")