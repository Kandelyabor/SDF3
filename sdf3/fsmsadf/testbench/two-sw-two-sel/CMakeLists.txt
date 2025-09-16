set(TEST_FILES 
    fsmsadf-two-sw-two-sel-mapped.xml.ref
    fsmsadf-two-sw-two-sel-merged.xml.ref
    fsmsadf-two-sw-two-sel.sdf
    fsmsadf-two-sw-two-sel.xml
    run_two-sw-two-sel.sh
    x4t1d-arch.xml
)

AddSDF3Test("fsmsadf_two_sw_two_sel" "${TEST_FILES}" sdf3-fsmsadf-two-sw-two-sel.opt)
