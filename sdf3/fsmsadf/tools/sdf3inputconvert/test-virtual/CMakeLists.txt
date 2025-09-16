set(TEST_FILES 
    example.xml
    fsmsadf-input-convert-virtual.xml.ref
)

AddSDF3Test("input_conversion_virtual" "${TEST_FILES}" sdf3-input-conversion-virtual.opt)

