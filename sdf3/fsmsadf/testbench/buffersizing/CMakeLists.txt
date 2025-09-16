set(TEST_FILES 
    simplefsmsadf.xml
    fsmsadf-buffersize.xml.ref
)

AddSDF3Test("fsmsadf-buffersize-maxplus" "${TEST_FILES}" sdf3-fsmsadf-buffersize-maxplus.opt)
