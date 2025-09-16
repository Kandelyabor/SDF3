set(TEST_FILES 
    app_test0.xml
	arch_buffdim.xml
	fsmsadf-input-convert-compsoc-1fsm.xml.ref
)

AddSDF3Test("input_conversion_compsoc_1fsm" "${TEST_FILES}" sdf3-input-conversion-compsoc-1fsm.opt)

