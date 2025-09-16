set(TEST_FILES
    simple_example.xml
    csdf-buffersize.xml.ref
)

AddSDF3Test("csdf-buffersize" "${TEST_FILES}"
    "sdf3-csdf-buffersize.opt"
)
