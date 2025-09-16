set(TEST_FILES
    simple_example.xml
    csdf-print-txt.txt.ref
)

AddSDF3Test("csdf-print-txt" "${TEST_FILES}"
    "sdf3-csdf-print-txt.opt"
)

set(TEST_FILES
    simple_example.xml
    csdf-print-buffy.c.ref
)

AddSDF3Test("csdf-print-buffy" "${TEST_FILES}"
    "sdf3-csdf-print-buffy.opt"
)

