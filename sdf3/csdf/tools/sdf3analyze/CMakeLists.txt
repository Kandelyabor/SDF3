add_executable(sdf3analyze-csdf
    sdf3analyze.cc
)

target_link_libraries(sdf3analyze-csdf base maxplus csdf ${LIBXML2_LIBRARIES})