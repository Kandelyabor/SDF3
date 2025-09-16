add_executable(sdf3print-sadf
    sdf3print.cc
)

target_link_libraries(sdf3print-sadf sadf base ${LIBXML2_LIBRARIES})