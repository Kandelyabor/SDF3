add_executable(sdf3transform-fsmsadf
    sdf3transform.cc
)

target_link_libraries(sdf3transform-fsmsadf fsmsadf base tools maxplus sdf ${LIBXML2_LIBRARIES})