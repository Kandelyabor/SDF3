set(TEST_FILES 
    fsmsadf-susan-mapped.xml.ref
    fsmsadf-susan-merged.xml.ref
    fsmsadf-susan.sdf
    fsmsadf-susan.xml
    run_susan-test.sh
    x4t1d-arch.xml
)

AddSDF3Test("fsmsadf_susan" "${TEST_FILES}" sdf3-fsmsadf-susan.opt)
