cmake_minimum_required(VERSION 3.22.0 FATAL_ERROR)
project(SDF3)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)

# TODO integrate cleanly only for MSVC/CLANG build
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

option(CODE_COVERAGE "Compile for code coverage (default OFF)." OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CPM_USE_LOCAL_PACKAGES ON)
include(config/get_cpm.cmake)

CPMAddPackage(
  GITHUB_REPOSITORY GNOME/LibXml2
  GIT_TAG v2.12.7
  VERSION 2.12.7
  OPTIONS "BUILD_SHARED_LIBS OFF"
  "LIBXML2_WITH_TESTS OFF"
  "LIBXML2_WITH_PROGRAMS OFF"
  "LIBXML2_WITH_ICONV OFF"
  "LIBXML2_WITH_ICU OFF"
  "LIBXML2_WITH_LZMA OFF"
  "LIBXML2_WITH_PYTHON OFF"
  "LIBXML2_WITH_ZLIB OFF"
  EXCLUDE_FROM_ALL YES
  SYSTEM YES
)

CPMAddPackage(
  GITHUB_REPOSITORY Model-Based-Design-Lab/maxpluslib
  GIT_TAG v2.2.1
  VERSION 2.2.1
  "CODE_COVERAGE OFF"
  "BUILD_TESTS OFF"

  EXCLUDE_FROM_ALL YES
  SYSTEM YES # Ignore warnings caused by the library
)


if(LibXml2_ADDED)
  if(NOT TARGET LibXml2::LibXml2)
    add_library(LibXml2::LibXml2 ALIAS LibXml2)
  endif()
  set(LIBXML2_INCLUDE_DIR ${LibXml2_SOURCE_DIR})
  list(APPEND DISABLE_WARNINGS_TARGETS LibXml2)

  if(TARGET LibXml2)
    # GCC/Clang: suppress common warnings for libxml2 only
    target_compile_options(LibXml2 PRIVATE
      -w # disables all warnings (use with care)
      # Or, selectively:
      # -Wno-deprecated-declarations
      # -Wno-sign-compare
      # -Wno-conversion
    )
  endif()
endif()


# set_property(LIBXML2_INCLUDE_DIR ${FETCHCONTENT_SOURCE_DIR_LIBXML3})
# find_package(LibXml2 REQUIRED)


# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Let's ensure -std=c++xx instead of -std=g++xx

  # # Set output folders
  set(SDF3_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
  set(SDF3_TEST_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/test)
  set(SDF3_TEST_REPORT_DIRECTORY ${SDF3_TEST_DIRECTORY}/xhtml)
  set(SDF3_TEST_RESULTS_DIRECTORY ${SDF3_TEST_DIRECTORY}/results)
  set(SDF3_COVERAGE_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/coverage)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SDF3_OUTPUT_DIRECTORY}/bin)


  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  find_package(Doxygen OPTIONAL_COMPONENTS dot)
  if (Doxygen_FOUND)
    add_subdirectory(documentation)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()


endif()

### CODE COVERAGE ###
if(${CODE_COVERAGE})
  message("Compiling for code coverage analysis.")
  set(CMAKE_BUILD_TYPE Debug)

  include(config/CodeCoverage.cmake)
  append_coverage_compiler_flags()
  # add_compile_options("-fprofile-arcs")
  # add_compile_options("-ftest-coverage")
endif()



enable_testing()

include(config/add_test.cmake)

add_custom_target(sdf3coverage_setup
  ${CMAKE_COMMAND} -E make_directory ${SDF3_COVERAGE_DIRECTORY}
)

add_custom_target(sdf3coverage1
  lcov  --capture --directory ${CMAKE_BINARY_DIR} --output ${SDF3_OUTPUT_DIRECTORY}/coverage.info --ignore-errors unused
  DEPENDS sdf3coverage_setup
)

add_custom_target(sdf3coverage2
  lcov -r ${SDF3_OUTPUT_DIRECTORY}/coverage.info /usr/include/\* -o ${SDF3_OUTPUT_DIRECTORY}/coverage.info --ignore-errors unused
  DEPENDS sdf3coverage1
)

add_custom_target(sdf3coverage
  genhtml ${SDF3_OUTPUT_DIRECTORY}/coverage.info --output-directory ${SDF3_COVERAGE_DIRECTORY}/
  DEPENDS sdf3coverage2
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


include (config/copy_examples_to_output.cmake)

add_subdirectory(base)
add_subdirectory(csdf)
add_subdirectory(fsmsadf)
add_subdirectory(sadf)
add_subdirectory(sdf)
add_subdirectory(tools)


