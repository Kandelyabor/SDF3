add_executable(sdf3convert-sadf-sdf2sadf
    sdf3convert.cc
)

target_link_libraries(sdf3convert-sadf-sdf2sadf sadf sdf base ${LIBXML2_LIBRARIES})