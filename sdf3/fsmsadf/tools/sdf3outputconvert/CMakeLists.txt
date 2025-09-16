add_subdirectory(test-compsoc-1scen)
add_subdirectory(test-virtual)

add_executable(sdf3outputconvert-fsmsadf
    sdf3outputconvert.cc
    settings.cc
)

target_link_libraries(sdf3outputconvert-fsmsadf fsmsadf base maxplus sdf ${LIBXML2_LIBRARIES})