include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRCS main.cpp udpsvc.cpp endpoint.cpp async.cpp nattransvc.cpp timer.cpp)
set(LONGOPT_SRCS longopt.cpp)

find_package(Protobuf REQUIRED)

message(STATUS ${PROTO_FILES})

foreach(file ${PROTO_FILES})
    execute_process(
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTO_FLAGS}
            --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto ${file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

file(GLOB_RECURSE PROTO_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/proto *.cc)

add_executable(nat_tran ${SRCS} ${PROTO_SRCS} ${LONGOPT_SRCS})
target_link_libraries(nat_tran libuv.a libprotobuf.a pthread)
