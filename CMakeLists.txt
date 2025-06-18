cmake_minimum_required(VERSION 3.30)
project(Easy_rider)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(HelloWorldApp STATIC
        src/HelloWorldApp.cpp
)
target_include_directories(HelloWorldApp
        PUBLIC ${CMAKE_SOURCE_DIR}/include
)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

target_link_libraries(HelloWorldApp
        PUBLIC
        sfml-graphics
        sfml-window
        sfml-system
)

add_executable(Easy_rider src/main.cpp)
target_link_libraries(Easy_rider
        PRIVATE HelloWorldApp
)

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

enable_testing()
add_subdirectory(tests)
