cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 14)
set(ROOT_PATH /zhuqs/zhuqs/mixdk)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_definitions(-g)
add_executable(
  mix_hash_unittest
  mix_hash_unittest.cpp
  ${ROOT_PATH}/src/mix_log.c
  ${ROOT_PATH}/src/mix_hash.c
)

add_executable(
  mix_bitmap_unittest
  mix_bitmap_unittest.cpp
  ${ROOT_PATH}/src/mix_log.c
  ${ROOT_PATH}/src/mix_bitmap.c
)

add_executable(
  mix_bloom_filter_unittest
  mix_bloom_filter_unittest.cpp
  ${ROOT_PATH}/src/mix_log.c
  ${ROOT_PATH}/src/mix_bloom_filter.c
  ${ROOT_PATH}/src/mix_bitmap.c
)

target_include_directories(
  mix_hash_unittest PUBLIC ${ROOT_PATH}/include
)

target_include_directories(
  mix_bitmap_unittest PUBLIC ${ROOT_PATH}/include
)

target_include_directories(
  mix_bloom_filter_unittest PUBLIC ${ROOT_PATH}/include
)

target_link_libraries(
  mix_hash_unittest
  gtest_main
)

target_link_libraries(
  mix_bitmap_unittest
  gtest_main
)

target_link_libraries(
  mix_bloom_filter_unittest
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(mix_hash_unittest mix_bitmap_unittest)