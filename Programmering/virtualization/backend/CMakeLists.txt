cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)

# Definer prosjektet
project(BackendApp)

# Legg til kildekoden
add_executable(cpp_backend app.cpp)  # Bytt ut main.cpp med ditt C++-filnavn
