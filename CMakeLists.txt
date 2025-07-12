cmake_minimum_required(VERSION 3.30)
project(Easy_rider LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE CORE_SOURCES
        ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_library(easy_rider_core STATIC
        ${CORE_SOURCES}
)

target_include_directories(easy_rider_core
        PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(easy_rider_core
        PUBLIC
        sfml-graphics
        sfml-window
        sfml-system
)

add_executable(Easy_rider
        ${CMAKE_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(Easy_rider
        PRIVATE
        easy_rider_core
)

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

#enable_testing()
#add_subdirectory(tests)
