set(TEST_FILES 
    fsmsadf-two-switches-mapped.xml.ref
    fsmsadf-two-switches-merged.xml.ref
    fsmsadf-two-switches.sdf
    fsmsadf-two-switches.xml
    run_two-switches.sh
    x4t1d-arch.xml
)

AddSDF3Test("fsmsadf_two_switches" "${TEST_FILES}" sdf3-fsmsadf-two-switches.opt)
