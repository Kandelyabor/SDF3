set(TEST_FILES 
    fsmsadf-output-convert-compsoc-1scen-mapped.xml
    architecture.xml
    fsmsadf-output-convert-compsoc-1scen-communication.xml.ref
    fsmsadf-output-convert-compsoc-1scen-mapping.xml.ref
)

AddSDF3Test("output_convert_compsoc_1scen" "${TEST_FILES}" sdf3-output-convert-compsoc-1scen.opt)
