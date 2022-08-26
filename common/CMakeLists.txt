set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable all warnings and treat them as errors
add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wconversion -Wformat=2 -Werror)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Wcast-align)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # gcc specific warnings
  add_compile_options(-Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wcast-align=strict -Warith-conversion)
endif()

if (ENABLE_STATIC_CHECKS)
  # enable clang-tidy and cppcheck checks
  find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy)
  if (CMAKE_CXX_CLANG_TIDY)
    string(CONCAT CLANG_TIDY_CHECKS
      "--checks="
      "-*,"
      "boost-*,"
      "bugprone-*,"
      "-bugprone-easily-swappable-parameters,"
      "cert-*,"
      "clang-analyzer-*,"
      "concurrency-*,"
      "cppcoreguidelines-*,"
      "-cppcoreguidelines-owning-memory,"
      "misc-*,"
      "-misc-non-private-member-variables-in-classes,"
      "modernize-*,"
      "-modernize-use-auto,"
      "-modernize-use-trailing-return-type,"
      "performance-*,"
      "portability-*,"
      "readability-*,"
      "-readability-avoid-const-params-in-decls,"
      "-readability-else-after-return,"
      "-readability-function-cognitive-complexity,"
      "-readability-identifier-length,"
      "-readability-implicit-bool-conversion"
    )
    list(
      APPEND CMAKE_CXX_CLANG_TIDY "--use-color" ${CLANG_TIDY_CHECKS})
  endif()

  find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
  if (CMAKE_CXX_CPPCHECK)
    list(
      APPEND CMAKE_CXX_CPPCHECK
        "--language=c++"
        "--std=c++17"
        "--enable=warning,performance"
    )
  endif()
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

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

