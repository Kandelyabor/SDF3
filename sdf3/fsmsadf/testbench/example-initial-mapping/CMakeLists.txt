set(TEST_FILES 
    duo-arch.xml
    fsmsadf-example-initial-mapping-mapped.xml.ref
    fsmsadf-example-initial-mapping.xml
)

AddSDF3Test("fsmsadf_example_initial_mapping" "${TEST_FILES}" sdf3-fsmsadf-example-initial-mapping.opt)
