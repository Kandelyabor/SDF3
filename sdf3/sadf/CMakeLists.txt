add_library(sadf)
include_directories(${LIBXML2_INCLUDE_DIR})

add_subdirectory(analysis)
add_subdirectory(base)
add_subdirectory(print)
add_subdirectory(simulation)
add_subdirectory(tools)
add_subdirectory(transformation)
add_subdirectory(verification)

add_subdirectory(testbench)

target_link_libraries(sadf fsmsadf csdf base)