set(CMAKE_C_STANDARD 11)

aux_source_directory(. SRC_LIST)

include_directories(../include)

add_definitions(-Wall -g -msse2)

add_executable(mixdk ${SRC_LIST})

target_link_libraries(mixdk pthread)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)