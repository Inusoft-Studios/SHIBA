add_executable(sandbox main.cpp)
target_link_libraries(sandbox shiba_platform)
shiba_enable_ipo(sandbox)
set_target_properties(sandbox PROPERTIES FOLDER "apps")