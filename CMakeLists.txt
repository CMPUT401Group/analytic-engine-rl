cmake_minimum_required(VERSION 3.0.2)
project(analyticenginerl)

include(CMakefiles/CMakeGenerateMainHeader)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -O3 -g")

# Include all header file to just one.
GenerateMainHeader(
        ${CMAKE_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/analytic-engine-cli.h
        ${CMAKE_SOURCE_DIR}/include
)
MESSAGE(STATUS "Generated global header file")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/build)

add_subdirectory(test)
add_subdirectory(src)

add_executable(analytic-engine-rl-cli main.cpp)
target_link_libraries(analytic-engine-rl-cli analyticenginerl)