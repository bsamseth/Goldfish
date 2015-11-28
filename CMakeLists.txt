cmake_minimum_required(VERSION 2.8.12)
project(goldfish)
 
enable_language(C CXX)

set(CMAKE_CXX_FLAGS "-std=c++0x -Wall")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")


#Bring the headers, such as Student.h into the project
include_directories(include)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.cpp")
 
add_executable(tester app/testing.cpp ${SOURCES})
add_executable(unittests app/unittests.cpp ${SOURCES})
