add_executable(sdf3transform-sdf
    sdf3transform.cc
)

target_link_libraries(sdf3transform-sdf base sdf ${LIBXML2_LIBRARIES})