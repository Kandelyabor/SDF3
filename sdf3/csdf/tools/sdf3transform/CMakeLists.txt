add_executable(sdf3transform-csdf
    sdf3transform.cc
)

target_link_libraries(sdf3transform-csdf base maxplus csdf fsmsadf ${LIBXML2_LIBRARIES})