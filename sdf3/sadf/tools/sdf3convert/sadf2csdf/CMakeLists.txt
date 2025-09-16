add_executable(sdf3convert-sadf-sadf2csdf
    sdf3convert.cc
)

target_link_libraries(sdf3convert-sadf-sadf2csdf sadf csdf base ${LIBXML2_LIBRARIES})