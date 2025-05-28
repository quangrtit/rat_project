cmake_minimum_required(VERSION 3.28)
project(Proto LANGUAGES CXX)

find_package(Protobuf REQUIRED)

set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Packet.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS} PROPERTIES GENERATED TRUE)

add_library(proto STATIC ${PROTO_SRCS})
target_include_directories(proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(proto PUBLIC ${Protobuf_LIBRARIES})

install(TARGETS proto DESTINATION lib)
install(FILES ${PROTO_HDRS} DESTINATION include/rat_proto)