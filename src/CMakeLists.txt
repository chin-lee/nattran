include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(PROTOBUF_INSTALL_DIR "C:\\protobuf")
    set(Protobuf_LIBRARIES "${PROTOBUF_INSTALL_DIR}\\lib")
    set(Protobuf_INCLUDE_DIR "${PROTOBUF_INSTALL_DIR}\\include")
    set(Protobuf_PROTOC_EXECUTABLE "${PROTOBUF_INSTALL_DIR}\\bin\\protoc.exe")
endif()

find_package(Protobuf REQUIRED)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    include_directories(${Protobuf_INCLUDE_DIRS})
    link_directories(${Protobuf_LIBRARIES})
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    include_directories("C:\\libuv\\include")
    if(CMAKE_BUILD_TYPE MATCHES DEBUG)
        link_directories("C:\\libuv\\Debug\\lib")
    else()
        link_directories("C:\\libuv\\Release\\lib")
    endif()
endif()

set(SRCS 
    main.cpp 
    udpsvc.cpp 
    endpoint.cpp 
    async.cpp 
    nattransvc.cpp 
    timer.cpp)
set(LONGOPT_SRCS longopt.cpp)

foreach(file ${PROTO_FILES})
    execute_process(
        COMMAND ${Protobuf_PROTOC_EXECUTABLE} ${PROTO_FLAGS}
            --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto ${file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

file(GLOB_RECURSE PROTO_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/proto *.cc)

add_executable(nat_tran 
    ${SRCS} 
    ${PROTO_SRCS} 
    ${LONGOPT_SRCS})

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    if(CMAKE_BUILD_TYPE MATCHES DEBUG)
        target_link_libraries(nat_tran libprotobufd.lib)
    else()
        target_link_libraries(nat_tran libprotobuf.lib)
    endif()
    target_link_libraries(nat_tran 
        libuv.lib
        ws2_32.lib
        psapi.lib
        iphlpapi.lib
        userenv.lib)
else()
    target_link_libraries(nat_tran 
        libuv.a 
        libprotobuf.a 
        pthread)
endif()