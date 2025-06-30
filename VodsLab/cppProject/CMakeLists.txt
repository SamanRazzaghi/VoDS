cmake_minimum_required(VERSION 3.10)

project(NetlistChecker VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES src/main.cpp)
add_executable(checker ${SOURCES})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(checker PRIVATE /W4 /permissive-)
elseif(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_options(checker PRIVATE -static -static-libgcc -static-libstdc++)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(checker PRIVATE -Wall -Wextra -Wpedantic)

endif()
