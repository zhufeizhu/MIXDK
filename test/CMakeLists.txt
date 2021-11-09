cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_STANDARD 11)

project(mixdk_test)

add_definitions(-g)

add_executable(nvm_test_read ./nvm_read.c)
add_executable(nvm_test_write ./nvm_write.c ./mix_queue.c)
add_executable(queue_test ./queue_test.c ./mix_queue.c)
add_executable(ssd_test_read ./test_ssd.c)

target_link_libraries(nvm_test_write pthread)
target_link_libraries(nvm_test_read pthread)
target_link_libraries(queue_test pthread)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)