set(TEST_FILES 
    fsmsadf-analyze-test-extractscenariographs.xml.ref
    fsmsadf-analyze-test.sdf
    fsmsadf-analyze-test.xml
)

AddSDF3Test("fsmsadf_extractscenariographs" "${TEST_FILES}" sdf3-analyze-extractscenariographs.opt)
