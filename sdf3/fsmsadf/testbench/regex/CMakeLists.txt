set(TEST_FILES 
    regexExample.xml
)

AddSDF3Test("fsmsadf_regex_latency" "${TEST_FILES}" sdf3-fsmsadf-latency-regex.opt)
AddSDF3Test("fsmsadf_regex_throughput" "${TEST_FILES}" sdf3-fsmsadf-throughput-regex.opt)
