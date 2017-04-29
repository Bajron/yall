cmake_minimum_required(VERSION 3.7)

project("yall")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(gmock)

enable_testing()

include_directories(include/)

add_executable(yall
  logger.cpp
)

add_executable(test_yall
  fmt.ut.cpp
)
target_link_libraries(test_yall gmock gtest gtest_main)

add_test(yall test_yall)
