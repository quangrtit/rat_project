cmake_minimum_required(VERSION 3.28)
project(Proto LANGUAGES CXX)


set(CMAKE_FIND_LIBRARY_SUFFIXES .a)


set(Protobuf_ROOT /usr/local/protobuf-static)
set(Protobuf_PROTOC_EXECUTABLE /usr/local/protobuf-static/bin/protoc)
set(Protobuf_INCLUDE_DIR /usr/local/protobuf-static/include)


find_package(Protobuf REQUIRED)
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf not found!")
else()
    message(STATUS "Protobuf found at: ${Protobuf_INCLUDE_DIRS}")
endif()


set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Packet.proto)
if(NOT EXISTS ${PROTO_FILES})
    message(FATAL_ERROR "Packet.proto not found at ${PROTO_FILES}")
endif()


protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS} PROPERTIES GENERATED TRUE)

add_library(proto STATIC ${PROTO_SRCS})

target_include_directories(proto PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}  
    ${Protobuf_INCLUDE_DIRS}     
)

target_link_libraries(proto PUBLIC ${Protobuf_LIBRARIES})

set_target_properties(proto PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON  
)

install(TARGETS proto DESTINATION lib)
install(FILES ${PROTO_HDRS} DESTINATION include/rat_proto)