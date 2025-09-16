
add_executable(sdf3test-run
runtest.cc
test_base.cc
test_command.cc
test_output_compare.cc
test_xml_compare.cc
)

target_link_libraries(sdf3test-run base tools maxplus ${LIBXML2_LIBRARIES})
