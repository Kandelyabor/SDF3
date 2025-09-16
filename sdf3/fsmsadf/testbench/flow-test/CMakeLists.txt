set(TEST_FILES 
    duo-arch.xml
    fsmsadf-flow-test-bag.xml.ref
    fsmsadf-flow-test-mapped.xml.ref
    fsmsadf-flow-test.sdf
    fsmsadf-flow-test.xml
    run_flow-test.sh
)

AddSDF3Test("fsmsadf_flow_test" "${TEST_FILES}" sdf3-fsmsadf-flow-test.opt)
