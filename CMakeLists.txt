cmake_minimum_required(VERSION 3.7)

project("yall")
add_subdirectory(googletest)
add_subdirectory(benchmark)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS_COVERAGE "-O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")

enable_testing()

include_directories(include benchmark/include)

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

add_executable(benchmark_yall
  logger.bm.cpp
)
target_link_libraries(benchmark_yall benchmark)

add_test(test_yall test_yall)
add_test(demo_yall demo_yall)
add_test(benchmark_yall benchmark_yall)
