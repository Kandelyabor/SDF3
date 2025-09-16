
add_library(csdf)
include_directories(${LIBXML2_INCLUDE_DIR})

add_subdirectory(analysis)
add_subdirectory(base)
add_subdirectory(generate)
add_subdirectory(output)
add_subdirectory(tools)
add_subdirectory(transform)

add_subdirectory(testbench)

target_link_libraries(csdf base fsmsadf)