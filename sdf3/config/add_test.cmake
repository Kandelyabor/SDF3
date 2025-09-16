# add test fixture

# setup create directories
add_test(NAME test_setup 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SDF3_TEST_RESULTS_DIRECTORY}
)

add_test(NAME test_make_report_dir 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SDF3_TEST_REPORT_DIRECTORY}
)

# teardown, run the tool te generate html summary
add_test(
    NAME test_teardown
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sdf3test-html --inputDirectory ${SDF3_TEST_RESULTS_DIRECTORY}/ --outputDirectory ${SDF3_TEST_REPORT_DIRECTORY}/
)

set_tests_properties(test_setup PROPERTIES FIXTURES_SETUP testFixture)
set_tests_properties(test_teardown PROPERTIES FIXTURES_CLEANUP testFixture)
set_tests_properties(test_make_report_dir PROPERTIES FIXTURES_SETUP testReporting)
set_tests_properties(test_teardown PROPERTIES FIXTURES_REQUIRED testReporting)

# add an SDF3 test with the given name, required files, and the opt files describing the tests
function(AddSDF3Test TEST_NAME TEST_FILES OPT_FILES)
    # Convert all files to absolute paths
    foreach(FILE IN LISTS TEST_FILES)
        file(REAL_PATH ${FILE} FILE_ABS) 
        list(APPEND TEST_FILES_ABS ${FILE_ABS})
    endforeach()
    # message("${TEST_FILES_ABS}")

    foreach(FILE IN LISTS OPT_FILES)
        file(REAL_PATH ${FILE} FILE_ABS)
        list(APPEND OPT_FILES_ABS ${FILE_ABS})
    endforeach()
    # message("${OPT_FILES_ABS}")

    add_test(NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND}
            "-DSDF3_TEST_FILES=${TEST_FILES_ABS}"
            "-DSDF3_OPT_FILES=${OPT_FILES_ABS}"
            "-DSDF3_TEST_EXE=$<TARGET_FILE:sdf3test-run>"
            -DSDF3_TEST_DIR=${SDF3_TEST_DIRECTORY}
            -DSDF3_TEST_RESULTS_DIR=${SDF3_TEST_RESULTS_DIRECTORY}
            -P ${CMAKE_SOURCE_DIR}/config/runtests.cmake)

    # message("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}")
    set_tests_properties(${TEST_NAME} PROPERTIES 
        FIXTURES_REQUIRED testFixture
        ENVIRONMENT_MODIFICATION "PATH=path_list_append:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG}"
    )

endfunction(AddSDF3Test)
