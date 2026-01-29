cmake_minimum_required(VERSION 3.10)
project(ParallelComputingAlgorithm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ParallelComputingAlgorithm
        main.cpp
        ThreadPool.cpp
)

target_include_directories(ParallelComputingAlgorithm PRIVATE .)

set(TXT_FILES
        file_test_1.txt
        file_test_2.txt
        file_test_3.txt
)

foreach(txt_file ${TXT_FILES})
    configure_file(${txt_file} ${CMAKE_CURRENT_BINARY_DIR}/${txt_file} COPYONLY)
endforeach()
