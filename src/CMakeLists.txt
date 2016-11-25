include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB SRC_FILES "*.cpp")

add_library(analyticenginerl ${SRC_FILES})
target_link_libraries(analyticenginerl rl)