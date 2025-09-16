set(TEST_FILES 
    architecture.xml
    bag-6.xml.ref
    fsmsadf-flow-test-mapped.xml.ref
    fsmsadf-flow-test.sdf
    fsmsadf-flow-test.xml
    fsmsadf-to_binding_aware.xml.ref
    run_test.sh
)

AddSDF3Test("fsmsadf_to_binding_aware" "${TEST_FILES}" sdf3-transform-to_binding_aware.opt)
