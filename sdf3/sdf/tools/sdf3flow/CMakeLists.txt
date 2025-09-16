add_executable(sdf3flow-sdf
    sdf3flow.cc
    settings.cc
)

target_link_libraries(sdf3flow-sdf base sdf ${LIBXML2_LIBRARIES})