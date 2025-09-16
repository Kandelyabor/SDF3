add_executable(sdf3analyze-sadf
    sdf3analyze.cc
)

target_link_libraries(sdf3analyze-sadf sadf base ${LIBXML2_LIBRARIES})