add_executable(sdf3generate-fsmsadf
    sdf3generate.cc
)

target_link_libraries(sdf3generate-fsmsadf fsmsadf base tools maxplus sdf ${LIBXML2_LIBRARIES})