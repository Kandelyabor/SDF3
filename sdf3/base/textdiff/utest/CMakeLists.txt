add_executable(textdiff-test
    textdiff.cc
    ../textdiff.cc
    ../../string/cstring.cc
)

target_include_directories(textdiff-test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


file(GLOB OPT_FILES files "${CMAKE_CURRENT_SOURCE_DIR}/*.opt")

set(TEST_FILES 
    a.txt
    a1.txt
    b.txt
    b1.txt
    empty.txt
)

AddSDF3Test("textdiff_utest" "${TEST_FILES}" "${OPT_FILES}")
