# Set the minimum version of CMake that can be used
cmake_minimum_required(VERSION 3.5.0)

#project name  
project(MY_THREAD_POOL)

#add compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall")

# set output binary path  
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

#head file pat
include_directories(${PROJECT_SOURCE_DIR}/include)

# define SRC_LIST 
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

# add executable file
add_executable(test_threadpool ${SRC_LIST})

# link the new test_connection target with the swap_lib target
