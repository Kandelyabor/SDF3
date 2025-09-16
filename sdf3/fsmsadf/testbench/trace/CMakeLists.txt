set(TEST_FILES 
    fsmsadf-analyze-test-trace.etf.ref
    fsmsadf-analyze-test-trace.txt.ref
    fsmsadf-analyze-test-trace-xml.xml.ref
    fsmsadf-analyze-test.sdf
    fsmsadf-analyze-test.xml
)

AddSDF3Test("fsmsadf_trace" "${TEST_FILES}" "sdf3-analyze-trace.opt;sdf3-analyze-trace-xml.opt")
