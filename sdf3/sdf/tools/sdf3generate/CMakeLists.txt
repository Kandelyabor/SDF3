add_executable(sdf3generate-sdf
    sdf3generate.cc
)

target_link_libraries(sdf3generate-sdf base sdf ${LIBXML2_LIBRARIES})