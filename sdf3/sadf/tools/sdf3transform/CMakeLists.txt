add_executable(sdf3transform-sadf
    sdf3transform.cc
)

target_link_libraries(sdf3transform-sadf sadf base ${LIBXML2_LIBRARIES})