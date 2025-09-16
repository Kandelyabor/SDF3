set(TEST_FILES 
    app_test0.xml
	arch_buffdim.xml
	fsmsadf-input-convert-compsoc.xml.ref
)

AddSDF3Test("input_conversion_compsoc" "${TEST_FILES}" sdf3-input-conversion-compsoc.opt)
