set(TEST_FILES 
    fsmsadf-three-sw-three-sel-mapped.xml.ref
    fsmsadf-three-sw-three-sel-merged.xml.ref
    fsmsadf-three-sw-three-sel.sdf
    fsmsadf-three-sw-three-sel.xml
    run_three-sw-three-sel.sh
    x4t1d-arch.xml
)

AddSDF3Test("fsmsadf_three_sw_three_sel" "${TEST_FILES}" sdf3-fsmsadf-three-sw-three-sel.opt)
