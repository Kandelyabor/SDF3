add_executable(sdf3generate-csdf
    sdf3generate.cc
)

target_link_libraries(sdf3generate-csdf base maxplus csdf sdf ${LIBXML2_LIBRARIES})