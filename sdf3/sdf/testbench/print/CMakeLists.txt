set(TEST_FILES 
    h263decoder.xml
    h263decoder.cc.ref
    h263decoder.dot.ref
    h263decoder.txt.ref
    h263decoder.buffy.cc.ref
    h263decoder.buffyThroughput.cc.ref
    h263decoder.stateSpaceThroughput.cc.ref
    h263decoder.selfTimedSchedule.cc.ref
    maxpluslinearsystem.txt.ref
    maxplusmatrix.txt.ref
    maxplusmatrix.latex.ref
    RFIDmachine.xml
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-sdf-print*.opt")
AddSDF3Test("sdf-print" "${TEST_FILES}" "${OPT_FILES}")

