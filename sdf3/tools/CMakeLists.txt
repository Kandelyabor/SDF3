add_library(tools)
include_directories(${LIBXML2_INCLUDE_DIR})

add_subdirectory(base)
add_subdirectory(test-html)
add_subdirectory(test-run)
add_subdirectory(test-settings)

target_link_libraries(tools base)