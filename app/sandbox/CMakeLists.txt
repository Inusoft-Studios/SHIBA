add_executable(sandbox main.cpp)
target_link_libraries(sandbox
        shiba::platform
        shiba::core
)
shiba_enable_ipo(sandbox)
set_target_properties(sandbox PROPERTIES FOLDER "apps")