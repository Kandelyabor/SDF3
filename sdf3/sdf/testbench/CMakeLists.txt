set(TEST_FILES
    h263decoder.xml
    sdf3-sdf-h263decoder-buffersize-output.xml.ref
    sdf3-sdf-h263decoder-mpconverthsdf-output.xml.ref
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-h263decoder*.opt")
AddSDF3Test("sdf-h263decoder" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    h263encoder.xml
    sdf3-sdf-h263encoder-buffersize.xml.ref
    sdf3-sdf-h263encoder-mpconverthsdf.xml.ref
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-h263encoder*.opt")
AddSDF3Test("sdf-h263encoder" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    modem.xml
    modem-mpdelay.xml
    sdf3-sdf-modem-buffersize.xml.ref
    sdf3-sdf-modem-mpconverthsdf.xml.ref
    sdf3-sdf-modem-retiming.xml.ref
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-modem*.opt")
AddSDF3Test("sdf-modem" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    mp3decoder_block_parallelism.xml
    sdf3-sdf-mp3decoder-block-parallelisml-buffersize.xml.ref
    sdf3-sdf-mp3decoder-block-parallelisml-mpconverthsdf.xml.ref
)

file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-mp3decoder-block-parallelism*.opt")
AddSDF3Test("sdf-mp3decoder-block-parallelism" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    mp3decoder_granule_parallelism.xml
    sdf3-sdf-mp3decoder-granule-parallelism-buffersize.xml.ref
    sdf3-sdf-mp3decoder-granule-parallelism-mpconverthsdf.xml.ref
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-mp3decoder-granule-parallelism*.opt")
AddSDF3Test("sdf-mp3decoder-granule-parallelism" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    mp3playback.xml
    sdf3-sdf-mp3playback-buffersize.xml.ref
    sdf3-sdf-mp3playback-mpconverthsdf.xml.ref
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-mp3playback*.opt")
AddSDF3Test("sdf-mp3playback" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    samplerate.xml
    sdf3-sdf-samplerate-buffersize.xml.ref
    sdf3-sdf-samplerate-mpconverthsdf.xml.ref
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-samplerate*.opt")
AddSDF3Test("sdf-samplerate" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    satellite.xml
    sdf3-sdf-satellite-buffersize.xml.ref
    sdf3-sdf-satellite-mpconverthsdf.xml.ref
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-satellite*.opt")
AddSDF3Test("sdf-satellite" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    OpenGraph.xml
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-opengraph*.opt")
AddSDF3Test("sdf-opengraph" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    hsdf.xml
    sdf3-hsdf-buffersize_capacity_constrained_output.xml.ref
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-hsdf*.opt")
AddSDF3Test("sdf-hsdf" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    simplehsdf.xml
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-simplehsdf*.opt")
AddSDF3Test("sdf-simplehsdf" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    inconsistent.xml
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-inconsistent*.opt")
AddSDF3Test("sdf-inconsistent" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    deadlock.xml
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-deadlock*.opt")
AddSDF3Test("sdf-deadlock" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
    stronglyconnected.xml
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-stronglyconnected*.opt")
AddSDF3Test("sdf-stronglyconnected" "${TEST_FILES}" "${OPT_FILES}")

set(TEST_FILES
float-execution-time.sdfx
)
file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-float*.opt")
AddSDF3Test("sdf-float" "${TEST_FILES}" "${OPT_FILES}")


add_subdirectory(flow)
add_subdirectory(print)