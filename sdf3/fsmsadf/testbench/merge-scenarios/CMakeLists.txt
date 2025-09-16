set(TEST_FILES 
    duo-arch.xml
    run_transform-test.sh
    transform-merge-scenarios-test-mapped.xml.ref
    transform-merge-scenarios-test-merged.xml.ref
    transform-merge-scenarios-test.sdf
    transform-merge-scenarios-test.xml
)

AddSDF3Test("fsmsadf_merge_scenarios" "${TEST_FILES}" sdf3-transform-merge-scenarios.opt)
