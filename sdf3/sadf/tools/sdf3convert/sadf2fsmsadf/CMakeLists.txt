add_executable(sdf3convert-sadf-sadf2fsmsadf
    sdf3convert.cc
)

target_link_libraries(sdf3convert-sadf-sadf2fsmsadf sadf fsmsadf base ${LIBXML2_LIBRARIES})