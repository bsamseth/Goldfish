# CMake config based on github.com/bsamseth/cpp-project

# This file specifies how the project should be built, using CMake.
# If you are unfamiliar with CMake, don't worry about all the details.
# The sections you might want to edit are marked as such, and 
# the comments should hopefully make most of it clear.
#
# For many purposes, you may not need to change anything about this file.

cmake_minimum_required(VERSION 3.0)

# Set project name here.
project(Goldfish)
enable_language(CXX)

# Set version number (change as needed). These definitions are available
# by including "projectmeta.hpp" in the source. 
# See projectmeta.hpp.in for some more details.
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 2)


# Include stuff. No change needed.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)


# --------------------------------------------------------------------------------
#                          Compile flags
# --------------------------------------------------------------------------------
# Set the C++ standard you wish to use (will apply to all files).
# If you do not use any features that limits the standard required,
# you could omit this line.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Things to always include as flags. Change as needed.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wfatal-errors")  

# Build-type specific flags. Change as needed.
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

message(STATUS "Building with the following extra flags: ${CMAKE_CXX_FLAGS}")

# --------------------------------------------------------------------------------
#                         Locate files 
# --------------------------------------------------------------------------------
# We make sure that CMake sees all the files.
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/tests
    ${PROJECT_SOURCE_DIR}/external/googletest
    ${PROJECT_SOURCE_DIR}/external/googletest/include)

# Make variables referring to all the sources and test files.  Could use file
# globbing, but this does not play nice with IDEs and is also discouraged due
# to a reload having to be done anyway when we add new files.
set(HEADERS
        include/LRUCache.hpp
        include/benchmark.hpp
        include/bitboard.hpp
        include/bound.hpp
        include/castling.hpp
        include/castlingtype.hpp
        include/color.hpp
        include/depth.hpp
        include/evaluation.hpp
        include/file.hpp
        include/goldfish.hpp
        include/move.hpp
        include/movegenerator.hpp
        include/movelist.hpp
        include/movetype.hpp
        include/notation.hpp
        include/perft.hpp
        include/piece.hpp
        include/piecetype.hpp
        include/position.hpp
        include/projectmeta.hpp.in
        include/protocol.hpp
        include/rank.hpp
        include/search.hpp
        include/square.hpp
        include/tt.hpp
        include/value.hpp)
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
        src/tt.cpp
        )
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
        tests/tt.cpp)
set(TEST_MAIN unit_tests.x)  # Default name for test executable (change if you wish).



# --------------------------------------------------------------------------------
#                            Build! 
# --------------------------------------------------------------------------------
# Compile all sources into a library. Called engine here (change if you wish).
add_library( engine ${SOURCES} ${HEADERS})

add_executable(goldfish.x app/goldfish.cpp)
add_dependencies(goldfish.x engine)
target_link_libraries(goldfish.x engine pthread)


# --------------------------------------------------------------------------------
#                         Make Tests
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
