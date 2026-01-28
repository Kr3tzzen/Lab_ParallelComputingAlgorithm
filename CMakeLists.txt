cmake_minimum_required(VERSION 3.10)
project(lab2)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

set(SOURCES
        main.cpp
)

add_executable(lab2 ${SOURCES})

target_link_libraries(lab2 Threads::Threads)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(lab2 PRIVATE -O2)
endif()

message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
