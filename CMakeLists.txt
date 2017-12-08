cmake_minimum_required(VERSION 2.8)
project(Goldfish)
 
enable_language(C CXX)


set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)

# --------------------------------------------------------------------------------
#                          Compile flags.
# --------------------------------------------------------------------------------
# These options can be turned on/off from the 'make' command.
option(debug "Use debugging flag" ON)
option(coverage "Use coverage flag" OFF)

# Set the C++ standard.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# Things to always include as flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")  

# Set debug/coverage flags, or optimize, depending on options.
if (debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    # Coverage can only be set if -g is used.
    if (coverage)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    endif()
else ()
    # Optimize flag for production code. Change as needed.
    set(CMAKE_CXX_FLAGS "-O2")
endif()

# --------------------------------------------------------------------------------
#                         Locate files.
# --------------------------------------------------------------------------------
# We make sure that CMake sees all the files.
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
    ${PROJECT_SOURCE_DIR}/external/googletest
    ${PROJECT_SOURCE_DIR}/external/googletest/include)

# Make variables referring to all the sources and test files.
file(GLOB SOURCES "src/*.cpp")
file(GLOB TESTFILES "tests/*.cpp")
set(TEST_MAIN unit_tests.x)

# --------------------------------------------------------------------------------
#                            Build! 
# --------------------------------------------------------------------------------
# Compile all sources into a library. Called engine here (change if you wish).
add_library( engine ${SOURCES} )

# Add an executables for main files in app/.
add_executable(tester app/testing.cpp)
add_dependencies(tester engine)
target_link_libraries(tester engine)

add_executable(goldfish app/goldfish.cpp)
add_dependencies(goldfish engine)
target_link_libraries(goldfish engine)


# --------------------------------------------------------------------------------
#                         Make Tests (no change needed).
# --------------------------------------------------------------------------------
# Add a make target 'gtest', that runs the tests (and builds all dependencies).
# The setup of Google Test is done at the very end of this file.
add_executable(${TEST_MAIN} ${TESTFILES})
add_dependencies(${TEST_MAIN} googletest engine)
target_link_libraries(${TEST_MAIN} googletest engine pthread)
add_custom_target(gtest 
    COMMAND "${PROJECT_BINARY_DIR}/${TEST_MAIN}"
    DEPENDS engine ${TEST_MAIN})


# Add a standard make target 'test' that runs the tests under CTest (only as an alt. to gtest).
include(CTest)
enable_testing()
add_test(unit_tests ${PROJECT_BINARY_DIR}/${TEST_MAIN})



# --------------------------------------------------------------------------------
#                         Documentation.
# --------------------------------------------------------------------------------
# Add a make target 'doc' to generate API documentation with Doxygen.
# You should set options to your liking in the file 'Doxyfile.in'.
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc ALL
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile &> doxygen.log
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
endif(DOXYGEN_FOUND)



# --------------------------------------------------------------------------------
#                         Google Test.
# --------------------------------------------------------------------------------
# The following makes sure that an up-to-date version of googletest is available,
# and built so that it may be used by your tests. 
add_custom_target( git_update
    COMMAND git submodule init
    COMMAND git submodule update
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} )
add_library( googletest
    ${PROJECT_SOURCE_DIR}/external/googletest/src/gtest-all.cc
    ${PROJECT_SOURCE_DIR}/external/googletest/src/gtest_main.cc )
add_dependencies(googletest git_update)
set_source_files_properties(${PROJECT_SOURCE_DIR}/external/googletest/src/gtest-all.cc  PROPERTIES GENERATED 1)
set_source_files_properties(${PROJECT_SOURCE_DIR}/external/googletest/src/gtest_main.cc PROPERTIES GENERATED 1)


# --------------------------------------------------------------------------------
#                            Misc (no change needed).
# --------------------------------------------------------------------------------
# Have CMake parse the config file, generating the config header, with
# correct definitions. Here only used to make version number available to
# the source code. Include "exampleConfig.h" (no .in suffix) in the source.
configure_file (
  "${PROJECT_SOURCE_DIR}/include/goldfish.h.in"
  "${PROJECT_BINARY_DIR}/goldfish.h")
# add the binary tree to the search path for include files
# so that we will find exampleConfig.h
include_directories("${PROJECT_BINARY_DIR}")
