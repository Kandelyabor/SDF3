set(TEST_FILES 
    arch.xml
    h263decoder.xml
    mp3decoder.xml
    h263decoder_result.xml.ref
    mp3decoder_result.xml.ref
    h263decoder_graph.html.ref
    mp3decoder_graph.html.ref
    sdf3.opt
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-sdf-flow*.opt")
AddSDF3Test("sdf-flow" "${TEST_FILES}" "${OPT_FILES}")

