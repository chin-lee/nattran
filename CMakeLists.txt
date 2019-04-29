cmake_minimum_required(VERSION 3.0)
project("nat_tran" CXX)

add_compile_options("-g" "-Wall" "-Wextra" "$<$<CONFIG:RELEASE>:-O2>")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options("-stdlib=libc++")
endif()

set(CMAKE_CXX_STANDARD 14)
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PROTO_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/proto)

file(GLOB_RECURSE PROTO_FILES proto *.proto)

add_subdirectory(src)
