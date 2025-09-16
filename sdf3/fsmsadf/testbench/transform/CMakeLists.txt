set(TEST_FILES 
    mp3decoder.xml
    mp3decoder_wc.xml.ref
    mp3decoder_autoconc.xml.ref
)

AddSDF3Test("fsmsadf-transform" "${TEST_FILES}" 
    "sdf3-fsmsadf-mp3decoder-wc_sdfg.opt;sdf3-fsmsadf-mp3decoder-model_autoconc.opt"
)

