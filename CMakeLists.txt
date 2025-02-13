cmake_minimum_required(VERSION 3.10)
project(IDATT2104)

set(CMAKE_CXX_STANDARD 14)

# Create executables for each file
add_executable(cplusplus_program Programmering/cplusplus.cpp)
add_executable(primenumber_program Programmering/primenumber.cpp)
add_executable(workers_program Programmering/workers.cpp)
