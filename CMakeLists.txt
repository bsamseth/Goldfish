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
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 2)


# Include stuff. No change needed.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)


# --------------------------------------------------------------------------------
#                          Compile flags (change as needed).
# --------------------------------------------------------------------------------
# Set the C++ standard you wish to use (will apply to all files).
# If you do not use any features that limits the standard required,
# you could omit this line.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Things to always include as flags. Change as needed.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")  

# Build-type specific flags. Change as needed.
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

message(STATUS "Building with the following extra flags: ${CMAKE_CXX_FLAGS}")

# --------------------------------------------------------------------------------
#                         Locate files (no change needed).
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
file(GLOB HEADERS "inlcude/*.hpp")
file(GLOB TESTFILES "tests/*.cpp")
set(TEST_MAIN unit_tests.x)  # Default name for test executable (change if you wish).



# --------------------------------------------------------------------------------
#                            Build! (Change as needed)
# --------------------------------------------------------------------------------
# Compile all sources into a library. Called engine here (change if you wish).
add_library( engine ${SOURCES} ${HEADERS})

add_executable(goldfish.x app/goldfish.cpp)
add_dependencies(goldfish.x engine)       
target_link_libraries(goldfish.x engine) 



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
#                         Code Coverage (no change needed).
# --------------------------------------------------------------------------------
# If CMake aborts due to missing dependencies for code coverage 
# (gcov, lcov, genhtml and supported compiler), comment this line.
include(CodeCoverage)

# --------------------------------------------------------------------------------
#                         Documentation (no change needed).
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
#                         Google Test (no change needed).
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
# the source code. Include "projectmeta.hpp" (no .in suffix) in the source.
configure_file (
  "${PROJECT_SOURCE_DIR}/include/projectmeta.hpp.in"
  "${PROJECT_BINARY_DIR}/projectmeta.hpp")
# add the binary tree to the search path for include files
# so that we will find projectmeta.hpp
include_directories("${PROJECT_BINARY_DIR}")

# Ask CMake to output a compile_commands.json file for use with things like Vim YCM.
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
