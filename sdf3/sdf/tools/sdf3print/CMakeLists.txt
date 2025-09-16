add_executable(sdf3print-sdf
    sdf3print.cc
)

target_link_libraries(sdf3print-sdf base maxplus sdf ${LIBXML2_LIBRARIES})