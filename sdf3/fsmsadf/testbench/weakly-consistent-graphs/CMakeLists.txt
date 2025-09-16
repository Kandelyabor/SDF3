set(TEST_FILES 
    mp3decoder_vld_r10_s5.xml
    mp3variant_wcfsmsadf.xml
    weaklyconsistent_extended.xml
    wlan_fsmsadf_manual.xml
)

file(GLOB OPT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdf3-*.opt")

AddSDF3Test("fsmsadf_weakly_consistent_graphs" "${TEST_FILES}" "${OPT_FILES}")
