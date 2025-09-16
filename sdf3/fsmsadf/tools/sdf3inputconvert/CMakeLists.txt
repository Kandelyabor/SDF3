add_subdirectory(test-compsoc)
add_subdirectory(test-compsoc-1fsm)
add_subdirectory(test-virtual)

add_executable(sdf3inputconvert-fsmsadf
    sdf3inputconvert.cc
    settings.cc
)

target_link_libraries(sdf3inputconvert-fsmsadf fsmsadf base maxplus sdf ${LIBXML2_LIBRARIES})