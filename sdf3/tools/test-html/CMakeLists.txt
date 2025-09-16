add_executable(sdf3test-html
    testhtml.cc
)

target_link_libraries(sdf3test-html base tools maxplus ${LIBXML2_LIBRARIES})
