target_sources(base PRIVATE
    dirs.cc
    shell.cc
)

if(UNIX)
    target_sources(base PRIVATE
        shellnix.cc
    )
elseif(WIN32)
    target_sources(base PRIVATE
        shellwin.cc
    )
else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif(UNIX)
