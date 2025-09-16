set(TEST_FILES 
    fsmsadf-one-switch-mapped.xml.ref
    fsmsadf-one-switch-merged.xml.ref
    fsmsadf-one-switch.sdf
    fsmsadf-one-switch.xml
    run_one-switch.sh
    x4t1d-arch.xml
)

AddSDF3Test("fsmsadf_one_switch" "${TEST_FILES}" sdf3-fsmsadf-one-switch.opt)
