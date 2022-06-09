cmake_minimum_required(VERSION 3.16)

# set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable all warnings and treat them as errors
add_compile_options(-Wall -Wextra -pedantic -Werror)
# enable clang-tidy checks, and tread warnings as errors
set(CMAKE_CXX_CLANG_TIDY clang-tidy)

# produce colored diagnostics when using Ninja
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-fdiagnostic-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

set(allowedBuildTypes Debug Release ASAN UBSAN TSAN)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" CACHE STRING "Debug build" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3" CACHE STRING "Release build" FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0" CACHE STRING "Address Sanitizer build" FORCE)
set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined -g -O0" CACHE STRING "Undefined Behaviour Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_TSAN "-fsanitize=thread -g -O0" CACHE STRING "Thread Sanitizer" FORCE)

project(cpp_concurrency
  VERSION 0.0.1
  DESCRIPTION "Examples of C++ concurrency related examples"
  LANGUAGES CXX)

find_package(Threads REQUIRED)

# executable using standard library shared_mutex (since C++17)
add_executable(05_read_write_lock 05_read_write_lock.cpp)
target_link_libraries(05_read_write_lock Threads::Threads)

# executable using Boost library shared_mutex (before C++17)
find_package(Boost
  CONFIG
  PATHS /u/gkan/remote_home/.local/boost-1.79.0
  COMPONENTS thread)
if(Boost_FOUND)
  message(STATUS ${Boost_VERSION_STRING})
  add_executable(06_read_write_lock_boost 06_read_write_lock_boost.cpp)
  target_include_directories(06_read_write_lock_boost PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(06_read_write_lock_boost Boost::thread)
endif()
