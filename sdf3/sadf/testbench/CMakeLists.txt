enable_testing()

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-compute-statistics" "${TEST_FILES}"
    "sdf3-sadf-example1-compute-statistics.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-compute-number-of-states" "${TEST_FILES}"
    "sdf3-sadf-example1-compute-number-of-states.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-compute-number-of-states-resolve-nondeterminism" "${TEST_FILES}"
    "sdf3-sadf-example1-compute-number-of-states-resolve-nondeterminism.opt"
)

set(TEST_FILES
    example2.xml
)

AddSDF3Test("sadf-compute-throughput" "${TEST_FILES}"
    "sdf3-sadf-example2-compute-throughput.opt"
)

set(TEST_FILES
    example2.xml
)

AddSDF3Test("sadf-compute-inter-firing-latency-minimum" "${TEST_FILES}"
    "sdf3-sadf-example2-compute-inter-firing-latency-minimum.opt"
)

set(TEST_FILES
    example2.xml
)

AddSDF3Test("sadf-compute-inter-firing-latency-variance" "${TEST_FILES}"
    "sdf3-sadf-example2-compute-inter-firing-latency-variance.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-compute-response-delay-maximum" "${TEST_FILES}"
    "sdf3-sadf-example1-compute-response-delay-maximum.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-compute-response-delay-expected" "${TEST_FILES}"
    "sdf3-sadf-example1-compute-response-delay-expected.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-compute-deadline-miss-probability" "${TEST_FILES}"
    "sdf3-sadf-example1-compute-deadline-miss-probability.opt"
)

set(TEST_FILES
    example2.xml
)

AddSDF3Test("sadf-compute-buffer-occupancy" "${TEST_FILES}"
    "sdf3-sadf-example2-compute-buffer-occupancy.opt"
)

set(TEST_FILES
    example2.xml
)

AddSDF3Test("sadf-compute-scenario-probabilities" "${TEST_FILES}"
    "sdf3-sadf-example2-compute-scenario-probabilities.opt"
)

set(TEST_FILES
    simple_example.xml
    simple_example2-sadf.xml.ref
)

AddSDF3Test("sadf-convert-csdf-to-sadf" "${TEST_FILES}"
    "sdf3-sadf-simple_example-convert-csdf-to-sadf.opt"
)

set(TEST_FILES
    ce_converted_csdf.xml
    sadf-to-csdf.xml.ref
)

AddSDF3Test("sadf-convert-sadf-to-csdf" "${TEST_FILES}"
    "sdf3-sadf-ce_converted_csdf-convert-sadf-to-csdf.opt"
)

set(TEST_FILES
    ce_converted_csdf_strongly_consistent.xml
    sadf-to-fsmsadf.xml.ref
)

AddSDF3Test("sadf-convert-sadf-to-fsmsadf" "${TEST_FILES}"
    "sdf3-sadf-ce_converted_csdf_strongly_consistent-convert-sadf-to-fsmsadf.opt"
)

set(TEST_FILES
    mp3_converted_sdf.xml
    sadf-to-sdf.xml.ref
)

AddSDF3Test("sadf-convert-sadf-to-sdf" "${TEST_FILES}"
    "sdf3-sadf-mp3_converted_sdf-convert-sadf-to-sdf.opt"
)

set(TEST_FILES
    h263decoder.xml
    sdf-to-sadf.xml.ref
)

AddSDF3Test("sadf-convert-sdf-to-sadf" "${TEST_FILES}"
    "sdf3-sadf-h263decoder-convert-sdf-to-sadf.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-print-dot" "${TEST_FILES}"
    "sdf3-sadf-example1-print-dot.opt"
)

set(TEST_FILES
    example1.xml
    example1.html.ref
)

AddSDF3Test("sadf-print-html" "${TEST_FILES}"
    "sdf3-sadf-example1-print-html.opt"
)

set(TEST_FILES
    example1.xml
    example1.php.ref
)

AddSDF3Test("sadf-print-php" "${TEST_FILES}"
    "sdf3-sadf-example1-print-php.opt"
)

set(TEST_FILES
    example1.xml
    example1.poosl.ref
)

AddSDF3Test("sadf-print-poosl" "${TEST_FILES}"
    "sdf3-sadf-example1-print-poosl.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-verify-connected-graph" "${TEST_FILES}"
    "sdf3-sadf-example1-verify-connected-graph.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-verify-is-timed" "${TEST_FILES}"
    "sdf3-sadf-example1-verify-is-timed.opt"
)

set(TEST_FILES
    example1.xml
)

AddSDF3Test("sadf-verify-strong-consistency" "${TEST_FILES}"
    "sdf3-sadf-example1-verify-strong-consistency.opt"
)

set(TEST_FILES
    example2.xml
)

AddSDF3Test("sadf-verify-moc" "${TEST_FILES}"
    "sdf3-sadf-example2-verify-moc.opt"
)

set(TEST_FILES
    example2.xml
)

AddSDF3Test("sadf-verify-is-hsdf" "${TEST_FILES}"
    "sdf3-sadf-example2-verify-is-hsdf.opt"
)

set(TEST_FILES
    mp3_converted_sdf.xml
)

AddSDF3Test("sadf-verify-is-sdf" "${TEST_FILES}"
    "sdf3-sadf-mp3_converted_sdf-verify-is-sdf.opt"
)

set(TEST_FILES
    mp3_converted_sdf.xml
)

AddSDF3Test("sadf-verify-is-csdf" "${TEST_FILES}"
    "sdf3-sadf-mp3_converted_sdf-verify-is-csdf.opt"
)

set(TEST_FILES
    mp3_converted_sdf.xml
)

AddSDF3Test("sadf-verify-simple-sdf-collection" "${TEST_FILES}"
    "sdf3-sadf-mp3_converted_sdf-verify-simple-sdf-collection.opt"
)

