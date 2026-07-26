# VMA is header-only
add_library(vma INTERFACE)
add_library(shiba::vma ALIAS vma)
target_include_directories(vma INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vma)

# VMA needs the Vulkan headers to compile.
find_package(Vulkan REQUIRED)
target_link_libraries(vma INTERFACE Vulkan::Headers)