set(TEST_FILES
    simple_example.xml
)

AddSDF3Test("csdf-analysis-consistency" "${TEST_FILES}"
    "sdf3-csdf-analysis-consistency.opt"
)

set(TEST_FILES
    simple_example.xml
)

AddSDF3Test("csdf-analysis-repetition-vector" "${TEST_FILES}"
    "sdf3-csdf-analysis-repetition-vector.opt"
)

set(TEST_FILES
    simple_example.xml
)

AddSDF3Test("csdf-analysis-repetition-vector-sum" "${TEST_FILES}"
    "sdf3-csdf-analysis-repetition-vector-sum.opt"
)

set(TEST_FILES
    simple_example.xml
)

AddSDF3Test("csdf-analysis-throughput" "${TEST_FILES}"
    "sdf3-csdf-analysis-throughput.opt"
)
