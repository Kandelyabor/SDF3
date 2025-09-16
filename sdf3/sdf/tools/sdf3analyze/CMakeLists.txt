add_executable(sdf3analyze-sdf
    sdf3analyze.cc
)

target_link_libraries(sdf3analyze-sdf base maxplus sdf ${LIBXML2_LIBRARIES})