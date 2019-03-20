# CMake config based on github.com/bsamseth/cpp-project

# This file specifies how the project should be built, using CMake.
# If you are unfamiliar with CMake, don't worry about all the details.
# The sections you might want to edit are marked as such, and 
# the comments should hopefully make most of it clear.
#
# For many purposes, you may not need to change anything about this file.

cmake_minimum_required(VERSION 3.0)

# Set project name here.
project(Goldfish VERSION 1.11.2 LANGUAGES C CXX)

# Include stuff. No change needed.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)


# Make variables referring to all the sources and test files.  Could use file
# globbing, but this does not play nice with IDEs and is also discouraged due
# to a reload having to be done anyway when we add new files.
set(SOURCES
        src/benchmark.cpp
        src/bitboard.cpp
        src/evaluation.cpp
        src/goldfish.cpp
        src/movegenerator.cpp
        src/movelist.cpp
        src/notation.cpp
        src/perft.cpp
        src/position.cpp
        src/search.cpp
        src/searchmanagement.cpp
        src/semaphore.cpp
        src/timer.cpp
        src/tb.cpp
        src/tt.cpp
        src/uci.cpp)
set(TESTFILES
        tests/bitboardtest.cpp
        tests/castlingtest.cpp
        tests/castlingtypetest.cpp
        tests/colortest.cpp
        tests/evaluationtest.cpp
        tests/filetest.cpp
        tests/movegeneratortest.cpp
        tests/movelisttest.cpp
        tests/movetest.cpp
        tests/notationtest.cpp
        tests/piecetest.cpp
        tests/piecetypetest.cpp
        tests/positiontest.cpp
        tests/ranktest.cpp
        tests/squaretest.cpp
        tests/tbtest.cpp
        tests/tt.cpp)
set(FATHOM_SOURCES
    external/Fathom/src/tbprobe.c)
set(TEST_MAIN unit_tests.x)


# --------------------------------------------------------------------------------
#                            Build! 
# --------------------------------------------------------------------------------
add_library(engine ${SOURCES} ${FATHOM_SOURCES})
target_include_directories(engine 
    PUBLIC ${PROJECT_SOURCE_DIR}/include 
           ${PROJECT_SOURCE_DIR}/external
           ${PROJECT_SOURCE_DIR}/external/Fathom/src)

target_compile_options(engine PRIVATE # clang/GCC warnings
                      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>: 
                      "-Wall" "-Wextra" >
                      # MSVC warnings
                      $<$<CXX_COMPILER_ID:MSVC>: 
                      "/Wall" >)

add_executable(goldfish.x app/goldfish.cpp)
target_link_libraries(goldfish.x PRIVATE engine $<$<BOOL:UNIX>:"pthread">)

add_executable(bench.x app/bench.cpp)
target_link_libraries(bench.x PRIVATE engine  $<$<BOOL:UNIX>:"pthread">)

add_executable(perft.x app/perft.cpp)
target_link_libraries(perft.x PRIVATE engine  $<$<BOOL:UNIX>:"pthread">)

set_target_properties(
    engine goldfish.x bench.x perft.x 
    PROPERTIES
    CXX_STANDARD 17 
    CXX_STANDARD_REQUIRED YES 
    CXX_EXTENSIONS NO)

# --------------------------------------------------------------------------------
#                         Make Tests
# --------------------------------------------------------------------------------
# Add a make target 'gtest', that runs the tests (and builds all dependencies).
# The setup of Google Test is done at the very end of this file.
add_executable(${TEST_MAIN} ${TESTFILES})
target_link_libraries(${TEST_MAIN} googletest engine  $<$<BOOL:UNIX>:"pthread">)
add_custom_target(gtest 
    COMMAND "${PROJECT_BINARY_DIR}/${TEST_MAIN}"
    DEPENDS ${TEST_MAIN})
add_custom_target(bench 
    COMMAND "${PROJECT_BINARY_DIR}/bench.x"
    DEPENDS engine bench.x)
add_custom_target(perft 
    COMMAND "${PROJECT_BINARY_DIR}/perft.x"
    DEPENDS engine perft.x)



# Add a standard make target 'test' that runs the tests under CTest (only as an alt. to gtest).
include(CTest)
enable_testing()
add_test(unit_tests ${PROJECT_BINARY_DIR}/${TEST_MAIN})

# --------------------------------------------------------------------------------
#                         Code Coverage 
# --------------------------------------------------------------------------------
# If CMake aborts due to missing dependencies for code coverage 
# (gcov, lcov, genhtml and supported compiler), comment this line.
include(CodeCoverage)

# --------------------------------------------------------------------------------
#                         Documentation 
# --------------------------------------------------------------------------------
# Add a make target 'doc' to generate API documentation with Doxygen.
# You should set options to your liking in the file 'Doxyfile.in'.
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc 
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile &> doxygen.log
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "${BoldMagenta}Generating API documentation with Doxygen (open ./html/index.html to view).${ColourReset}" VERBATIM
        )
endif(DOXYGEN_FOUND)


# --------------------------------------------------------------------------------
#                         Google Test
# --------------------------------------------------------------------------------
add_library( googletest
    ${PROJECT_SOURCE_DIR}/external/googletest/googletest/src/gtest-all.cc
    ${PROJECT_SOURCE_DIR}/external/googletest/googletest/src/gtest_main.cc
)
target_include_directories(googletest 
    PUBLIC ${PROJECT_SOURCE_DIR}/external/googletest/googletest
           ${PROJECT_SOURCE_DIR}/external/googletest/googletest/src
           ${PROJECT_SOURCE_DIR}/external/googletest/googletest/include)
set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/external/googletest/src/gtest-all.cc  
    ${PROJECT_SOURCE_DIR}/external/googletest/src/gtest_main.cc
    PROPERTIES GENERATED 1
)

# --------------------------------------------------------------------------------
#                            Misc
# --------------------------------------------------------------------------------
# Have CMake parse the config file, generating the config header, with
# correct definitions. Here only used to make version number available to
# the source code. Include "projectmeta.hpp" (no .in suffix) in the source.
configure_file (
  "${PROJECT_SOURCE_DIR}/include/projectmeta.hpp.in"
  "${PROJECT_BINARY_DIR}/projectmeta.hpp")
# add the binary tree to the search path for include files
# so that we will find projectmeta.hpp
include_directories("${PROJECT_BINARY_DIR}")

# Ask CMake to output a compile_commands.json file for use with things like Vim YCM.
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
