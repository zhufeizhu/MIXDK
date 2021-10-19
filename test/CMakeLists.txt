cmake_minimum_required(VERSION 2.8)

project(mixdk_test)

add_definitions(-g)

add_executable(nvm_test_read ./nvm_read.c)
add_executable(nvm_test_write ./nvm_write.c ./mix_queue.c)

target_link_libraries(nvm_test_write pthread)
target_link_libraries(nvm_test_read pthread)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)