add_executable(sdf3verify-sadf
    sdf3verify.cc
)

target_link_libraries(sdf3verify-sadf sadf base ${LIBXML2_LIBRARIES})