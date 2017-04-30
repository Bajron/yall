cmake_minimum_required(VERSION 3.7)

project("yall")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

message(${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS_COVERAGE "-O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")

add_subdirectory(gmock)

enable_testing()

include_directories(include/)

add_executable(demo_yall
  logger.mt.cpp
)

add_executable(test_yall
  fmt.ut.cpp
  priority.ut.cpp
  logger.ut.cpp
  backends.ut.cpp
  prefix.ut.cpp
)
target_link_libraries(test_yall gmock gtest gtest_main)

add_test(test_yall test_yall)
add_test(demo_yall demo_yall)
