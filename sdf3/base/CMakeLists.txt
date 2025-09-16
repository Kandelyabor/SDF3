
add_library(base)
target_link_libraries(base maxplus LibXml2)

target_include_directories(base
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${LIBXML2_INCLUDE_DIR}
)
include_directories(${LIBXML2_INCLUDE_DIR})

add_subdirectory(exception)
add_subdirectory(log)
add_subdirectory(matrix)
add_subdirectory(shell)
add_subdirectory(string)
add_subdirectory(tempfile)
add_subdirectory(textdiff)
add_subdirectory(time)
add_subdirectory(xhtml)
add_subdirectory(xml)
add_subdirectory(xmlcompare)
