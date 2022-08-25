cmake_minimum_required(VERSION 3.20)
project(00_examples)

include(${CMAKE_CURRENT_SOURCE_DIR}/../common/CMakeLists.txt)

add_executable(01_print_in_order 01_print_in_order.cpp)
target_link_libraries(01_print_in_order Threads::Threads)
