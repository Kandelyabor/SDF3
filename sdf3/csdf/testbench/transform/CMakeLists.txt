set(TEST_FILES
    simple_example.xml
    csdf-transform-wcfsmsadf.xml.ref
)

AddSDF3Test("csdf-transform-wcfsmsadf" "${TEST_FILES}"
    "sdf3-csdf-transform-wcfsmsadf.opt"
)

