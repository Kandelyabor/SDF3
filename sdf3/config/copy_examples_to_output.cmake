set(CMAKE_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples)
set(CMAKE_SDF_FLOW_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples/sdf-flow)
set(CMAKE_SDF_GENERATE_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples/sdf-generate)
set(CMAKE_CSDF_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples/csdf)
set(CMAKE_CSDF_GENERATE_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples/csdf-generate)
set(CMAKE_FSMSADF_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples/fsmsadf)
set(CMAKE_FSMSADF_FLOW_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples/fsmsadf-flow)
set(CMAKE_FSMSADF_INPUT_OUTPUT_CONVERT_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples/fsmsadf-input-output-convert)
set(CMAKE_SADF_EXAMPLES_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/examples/sadf)


add_custom_target(make_example_directories ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SDF_FLOW_EXAMPLES_OUTPUT_DIRECTORY}
  COMMENT "Creating example file directories"
)

# Copy example files from sdf/testbench to ${SDF3_OUTPUT_DIRECTORY}/examples
file(GLOB SDF_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/h263decoder.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/satellite.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/hsdf.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/simplehsdf.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/stronglyconnected.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/modem-mpdelay.xml"
)

add_custom_target(copy_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SDF_EXAMPLE_FILES}
    ${CMAKE_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${SDF_EXAMPLE_FILES}
  COMMENT "Copying example files to output directory"
)

# Copy flow example files from sdf/testbench/flow to ${SDF3_OUTPUT_DIRECTORY}/examples/flow
file(GLOB SDF_FLOW_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/flow/h263decoder.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/flow/h263decoder_result.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/flow/mp3decoder.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/flow/sdf3.opt"
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/flow/arch.xml"
)

add_custom_target(copy_flow_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SDF_FLOW_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SDF_FLOW_EXAMPLE_FILES}
    ${CMAKE_SDF_FLOW_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${SDF_FLOW_EXAMPLE_FILES}
  COMMENT "Copying flow example files to output directory"
)

# Copy generate example files from sdf/testbench/generate to ${SDF3_OUTPUT_DIRECTORY}/examples/generate
file(GLOB SDF_GENERATE_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/sdf/testbench/generate/sdf3.opt"
)

add_custom_target(copy_generate_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SDF_GENERATE_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SDF_GENERATE_EXAMPLE_FILES}
    ${CMAKE_SDF_GENERATE_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${SDF_GENERATE_EXAMPLE_FILES}
  COMMENT "Copying generate example files to output directory"
)

# Copy csdf example file from csdf/testbench/buffersizing to ${SDF3_OUTPUT_DIRECTORY}/examples/csdf
file(GLOB CSDF_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/csdf/testbench/buffersizing/simple_example.xml"
)

add_custom_target(copy_csdf_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CSDF_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CSDF_EXAMPLE_FILES}
    ${CMAKE_CSDF_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${CSDF_EXAMPLE_FILES}
  COMMENT "Copying CSDF example files to output directory"
)

# Copy generate example files from csdf/testbench/generate to ${SDF3_OUTPUT_DIRECTORY}/examples/csdf-generate
file(GLOB CSDF_GENERATE_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/csdf/testbench/generate/sdf3.opt"
)

add_custom_target(copy_csdf_generate_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CSDF_GENERATE_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CSDF_GENERATE_EXAMPLE_FILES}
    ${CMAKE_CSDF_GENERATE_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${CSDF_GENERATE_EXAMPLE_FILES}
  COMMENT "Copying csdf generate example files to output directory"
)


# Copy example files from fsmsadf/testbench to ${SDF3_OUTPUT_DIRECTORY}/examples/fsmsadf
file(GLOB FSMSADF_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/testbench/analysis/mp3decoder.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/testbench/analysis/bag-1.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/testbench/analysis/bag-13.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/testbench/analysis/fsmsadf-susan.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/testbench/regex/regexExample.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/testbench/buffersizing/simplefsmsadf.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/testbench/transform/fsmsadf-flow-test-mapped.xml"
)

add_custom_target(copy_fsmsadf_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_FSMSADF_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${FSMSADF_EXAMPLE_FILES}
    ${CMAKE_FSMSADF_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${FSMSADF_EXAMPLE_FILES}
  COMMENT "Copying FSMSADF example files to output directory"
)

# Copy flow example files from sdf/testbench/flow to ${SDF3_OUTPUT_DIRECTORY}/examples/flow
file(GLOB FSMSADF_FLOW_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/testbench/flow-test/sdf3-fsmsadf-flow-test.opt"
)

add_custom_target(copy_fsmsadf_flow_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_FSMSADF_FLOW_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${FSMSADF_FLOW_EXAMPLE_FILES}
    ${CMAKE_FSMSADF_FLOW_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${FSMSADF_FLOW_EXAMPLE_FILES}
  COMMENT "Copying fsmsadf flow example files to output directory"
)

# Copy executable files to ${SDF3_OUTPUT_DIRECTORY}/bin
file(GLOB SDF3_DIST_EXECUTABLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/generate/gen_appxml.py"
)

add_custom_target(copy_additional_sdf3_executable_files ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SDF3_DIST_EXECUTABLE_FILES}
    ${SDF3_OUTPUT_DIRECTORY}/bin
  DEPENDS
    ${SDF3_DIST_EXECUTABLE_FILES}
  COMMENT "Copying additional executables to the output bin directory"
)

# Copy input convert example files from fsmsadf/tools/inputconvert/examples to ${SDF3_OUTPUT_DIRECTORY}/examples/fsmsadf-input-convert
file(GLOB FSMSADF_INPUT_OUTPUT_CONVERT_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/tools/sdf3inputconvert/examples/sdf3-input-conversion-compsoc.opt"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/tools/sdf3inputconvert/examples/app_test0.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/tools/sdf3inputconvert/examples/arch_buffdim.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/tools/sdf3outputconvert/examples/sdf3-output-convert-compsoc-1scen.opt"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/tools/sdf3outputconvert/examples/fsmsadf-output-convert-compsoc-1scen-mapped.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/fsmsadf/tools/sdf3outputconvert/examples/architecture.xml"
)

add_custom_target(copy_fsmsadf_input_output_convert_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_FSMSADF_INPUT_OUTPUT_CONVERT_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${FSMSADF_INPUT_OUTPUT_CONVERT_EXAMPLE_FILES}
    ${CMAKE_FSMSADF_INPUT_OUTPUT_CONVERT_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${FSMSADF_INPUT_OUTPUT_CONVERT_EXAMPLE_FILES}
  COMMENT "Copying fsmsadf input output convert example files to output directory"
)

# Copy sadf example files from sadf/testbench/ to ${SDF3_OUTPUT_DIRECTORY}/examples/sadf
file(GLOB SADF_EXAMPLE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/sadf/testbench/example1.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sadf/testbench/example2.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sadf/testbench/ce_converted_csdf.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sadf/testbench/ce_converted_csdf_strongly_consistent.xml"
  "${CMAKE_CURRENT_SOURCE_DIR}/sadf/testbench/mp3_converted_sdf.xml"
)

add_custom_target(copy_sadf_example_files ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SADF_EXAMPLES_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SADF_EXAMPLE_FILES}
    ${CMAKE_SADF_EXAMPLES_OUTPUT_DIRECTORY}
  DEPENDS
    make_example_directories
    ${SADF_EXAMPLE_FILES}
  COMMENT "Copying sadf example files to output directory"
)
