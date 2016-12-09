cmake_minimum_required(VERSION 2.8)
project(goldfish)
 
enable_language(C CXX)


set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -O0 -g3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")


include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/googletest
    ${PROJECT_SOURCE_DIR}/external/googletest/include
    )

file(GLOB SOURCES "src/*.cpp")
file(GLOB TESTFILES "tests/*.cpp")

add_library(
    engine
    ${SOURCES}
    )

add_custom_target(
    git_update
    COMMAND git submodule init
    COMMAND git submodule update
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

add_library(
    gtest
    ${PROJECT_SOURCE_DIR}/external/googletest/src/gtest-all.cc
    ${PROJECT_SOURCE_DIR}/external/googletest/src/gtest_main.cc
    )

add_dependencies(gtest git_update)

set_source_files_properties(${PROJECT_SOURCE_DIR}/external/googletest/src/gtest-all.cc  PROPERTIES GENERATED 1)
set_source_files_properties(${PROJECT_SOURCE_DIR}/external/googletest/src/gtest_main.cc PROPERTIES GENERATED 1)

add_executable(
    unit_tests
    ${TESTFILES}
    )

add_dependencies(unit_tests gtest)

target_link_libraries(
    unit_tests
    gtest
    engine
    pthread
    )

 
add_executable(tester app/testing.cpp)

add_dependencies(tester engine)

target_link_libraries(tester engine)

add_executable(goldfish app/goldfish.cpp)

add_dependencies(goldfish engine)

target_link_libraries(goldfish engine)

add_custom_target(
    coverage
    COMMAND cd ${PROJECT_SOURCE_DIR}/build
    COMMAND lcov --directory ${PROJECT_SOURCE_DIR}/build --capture --output-file coverage.info # capture coverage info
    COMMAND lcov --remove coverage.info 'tests/*' 'app/*' 'external/*' 'cmake/*' '/usr/*' --output-file coverage.info 
    COMMAND lcov --list coverage.info
    COMMAND genhtml coverage.info
    COMMAND google-chrome-stable index.html&
    COMMAND cd -
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

add_dependencies(coverage unit_tests)

include(CTest)
enable_testing()

add_test(unit ${PROJECT_BINARY_DIR}/unit_tests)
