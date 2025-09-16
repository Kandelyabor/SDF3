set(TEST_FILES 
    fsmsadf-sw-sel-mapped.xml.ref
    fsmsadf-sw-sel-merged.xml.ref
    fsmsadf-sw-sel.sdf
    fsmsadf-sw-sel.xml
    run_sw-sel.sh
    x4t1d-arch.xml
)

AddSDF3Test("fsmsadf_switch_select" "${TEST_FILES}" sdf3-fsmsadf-sw-sel.opt)
