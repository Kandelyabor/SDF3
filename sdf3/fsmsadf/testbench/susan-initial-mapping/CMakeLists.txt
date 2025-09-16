set(TEST_FILES 
    fsmsadf-susan-initial-mapping-mapped.xml.ref
    fsmsadf-susan-initial-mapping-merged.xml.ref
    fsmsadf-susan-initial-mapping.sdf
    fsmsadf-susan-initial-mapping.xml
    run_susan-test.sh
    x4t1d-arch.xml
)

AddSDF3Test("fsmsadf_susan_initial_mapping" "${TEST_FILES}" sdf3-fsmsadf-susan-initial-mapping.opt)
