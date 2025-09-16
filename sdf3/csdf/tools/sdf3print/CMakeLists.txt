add_executable(sdf3print-csdf
    sdf3print.cc
)

target_link_libraries(sdf3print-csdf base maxplus csdf ${LIBXML2_LIBRARIES})