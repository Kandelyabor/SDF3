# message("Running test!")
# message(${SDF3_TEST_FILES})
# message(${SDF3_TEST_DIR})
# message(${SDF3_TEST_RESULTS_DIR})
# message(${SDF3_OPT_FILES})

set(validator ${SDF3_TEST_EXE})

foreach(filename ${SDF3_TEST_FILES})
    # message("File:  ${filename}")
    # message("Destintion:  ${SDF3_TEST_DIR}")
    file(COPY ${filename} DESTINATION ${SDF3_TEST_DIR})
endforeach()

set(SDF3_SOME_TEST_FAILED FALSE)

foreach(filename ${SDF3_OPT_FILES})
    # message("Opt File:  ${filename}")
    file(COPY ${filename} DESTINATION ${SDF3_TEST_DIR})
    # message("Opt File:  ${filename}")
    get_filename_component(outputfilename ${filename} NAME_WLE)
    # message("Xml File:  ${outputfilename}")
    execute_process(
        COMMAND "${validator}" --settings ${filename} --output ${SDF3_TEST_RESULTS_DIR}/${outputfilename}.xml
        WORKING_DIRECTORY ${SDF3_TEST_DIR}
        RESULT_VARIABLE SDF3_TEST_RESULT
    )
    if(NOT (${SDF3_TEST_RESULT} EQUAL 0))
        message("Test has failed: ${SDF3_TEST_RESULT}")
        message("For details, see: ${SDF3_TEST_RESULTS_DIR}/${outputfilename}.xml")
        set(SDF3_SOME_TEST_FAILED TRUE)
    endif(NOT (${SDF3_TEST_RESULT} EQUAL 0))
    
endforeach()

# clean up all files in test directory
file(GLOB FILES_TO_DELETE files "${SDF3_TEST_DIR}/*")

foreach(filename ${FILES_TO_DELETE})
    # message("Removing:  ${filename}")
    file(REMOVE ${filename})
endforeach()

if (SDF3_SOME_TEST_FAILED)
    message(FATAL_ERROR "A test failed.")
endif(SDF3_SOME_TEST_FAILED)

