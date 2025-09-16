add_executable(sdf3flow-fsmsadf
    sdf3flow.cc
    settings.cc
)

target_link_libraries(sdf3flow-fsmsadf fsmsadf base maxplus sdf ${LIBXML2_LIBRARIES})