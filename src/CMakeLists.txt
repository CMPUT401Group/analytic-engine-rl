include_directories(${analyticenginerl_SOURCE_DIR}/include)

file(GLOB SRC_FILES "*.cpp")
add_library(analyticenginerl ${SRC_FILES})
target_link_libraries(analyticenginerl rl)