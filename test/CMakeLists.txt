include_directories(${analyticenginerl_SOURCE_DIR}/include)
include_directories(${analyticenginerl_SOURCE_DIR}/test/include)

file(GLOB SRC_TEST_FILES "src/*.cpp")

add_executable(testExecutable test-runner.cpp ${SRC_TEST_FILES})
target_link_libraries(testExecutable rl)