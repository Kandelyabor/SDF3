add_executable(sdf3convert-sadf-csdf2sadf
    sdf3convert.cc
)

target_link_libraries(sdf3convert-sadf-csdf2sadf sadf csdf base ${LIBXML2_LIBRARIES})