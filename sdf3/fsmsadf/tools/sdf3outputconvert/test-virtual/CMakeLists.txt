set(TEST_FILES 
    input.xml
    fsmsadf-output-convert-virtual.xml.ref
)

AddSDF3Test("output_conversion_virtual" "${TEST_FILES}" sdf3-output-conversion-virtual.opt)
