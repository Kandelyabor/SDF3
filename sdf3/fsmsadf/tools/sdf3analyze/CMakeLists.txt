add_executable(sdf3analyze-fsmsadf
    sdf3analyze.cc
)

target_link_libraries(sdf3analyze-fsmsadf fsmsadf base maxplus sdf tools ${LIBXML2_LIBRARIES})
