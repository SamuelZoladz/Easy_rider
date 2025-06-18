cmake_minimum_required(VERSION 3.30)
project(Easy_rider)

set(CMAKE_CXX_STANDARD 20)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(Easy_rider main.cpp)
target_link_libraries(Easy_rider sfml-graphics sfml-window sfml-system)