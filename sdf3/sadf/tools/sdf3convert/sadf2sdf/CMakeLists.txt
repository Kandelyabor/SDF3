add_executable(sdf3convert-sadf-sadf2sdf
    sdf3convert.cc
)

target_link_libraries(sdf3convert-sadf-sadf2sdf sadf sdf base ${LIBXML2_LIBRARIES})