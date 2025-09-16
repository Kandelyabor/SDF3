set(TEST_FILES
    mp3decoder.xml
    fsmsadf-susan.xml
)

AddSDF3Test("fsmsadf-analysis-throughput" "${TEST_FILES}"
    "sdf3-fsmsadf-mp3decoder-throughput-mpautomaton.opt;sdf3-fsmsadf-mp3decoder-throughput.opt;sdf3-fsmsadf-mp3decoder-throughput-statespace.opt"
)

set(TEST_FILES
    mp3decoder.xml
    sdf3-fsmsadf-mp3decoder-isolatescenarios-result.xml.ref
)

AddSDF3Test("fsmsadf-analysis-isolatescenarios" "${TEST_FILES}"
    sdf3-fsmsadf-mp3decoder-isolatescenarios.opt
)

set(TEST_FILES
    fsmsadf-susan.xml
    sdf3-fsmsadf-mp3decoder-criticalscenariosequence-result.xml.ref
)

AddSDF3Test("fsmsadf-analysis-criticalscenariosequence" "${TEST_FILES}"
    sdf3-fsmsadf-mp3decoder-criticalscenariosequence.opt
)

set(TEST_FILES
    mp3decoder.xml
)

AddSDF3Test("fsmsadf-analysis-mpeigen" "${TEST_FILES}"
    sdf3-fsmsadf-mp3decoder-mpeigen.opt
)

set(TEST_FILES
    mp3decoder.xml
    sdf3-fsmsadf-mp3decoder-vectortrace-result.xml.ref
)

AddSDF3Test("fsmsadf-analysis-vectortrace" "${TEST_FILES}"
    sdf3-fsmsadf-mp3decoder-vectortrace.opt
)

set(TEST_FILES
    bag-1.xml
)

AddSDF3Test("fsmsadf-analysis-schedule-check" "${TEST_FILES}"
    sdf3-fsmsadf-bag1-schedule-check.opt
)
