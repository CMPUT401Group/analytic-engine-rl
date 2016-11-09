cmake_minimum_required(VERSION 3.0.2)
project(analyticenginerl)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -O0 -g")

#add_subdirectory(src)
add_subdirectory(test)

include_directories(${analyticenginerl_SOURCE_DIR}/include)

add_executable(analyticenginerl-cli main.cpp)
target_link_libraries(analyticenginerl-cli rl)