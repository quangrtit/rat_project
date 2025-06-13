cmake_minimum_required(VERSION 3.28)
project(RATServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(BUILD_SHARED_LIBS OFF)

set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT /usr/local)
set(BOOST_LIBRARYDIR /usr/local/lib)
find_package(Boost 1.83 REQUIRED COMPONENTS system thread)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found! Please check the installation or compile with static libraries.")
endif()
message(STATUS "Boost found at: ${Boost_INCLUDE_DIRS} - Libraries: ${Boost_LIBRARIES}")

set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_ROOT_DIR /usr/local/openssl-3.0.14)
set(OPENSSL_INCLUDE_DIR /usr/local/openssl-3.0.14/include)
set(OPENSSL_SSL_LIBRARY /usr/local/openssl-3.0.14/lib64/libssl.a)
set(OPENSSL_CRYPTO_LIBRARY /usr/local/openssl-3.0.14/lib64/libcrypto.a)

set(Protobuf_ROOT /usr/local/protobuf-static)
set(Protobuf_PROTOC_EXECUTABLE /usr/local/protobuf-static/bin/protoc)
set(Protobuf_INCLUDE_DIR /usr/local/protobuf-static/include)
set(Protobuf_LIBRARY /usr/local/protobuf-static/lib/libprotobuf.a)
find_package(Protobuf REQUIRED)
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf not found!")
endif()
message(STATUS "Protobuf found at: ${Protobuf_INCLUDE_DIRS}")

set(SERVER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMMON_DIR ${CMAKE_SOURCE_DIR}/../common)

add_executable(rat_server
    src/main.cpp
    src/Server.cpp
    src/ServerGUI.cpp
    src/Utils.cpp
    src/FileReceiver.cpp
    src/ProcessHandler.cpp
    src/FilesFoldersHandler.cpp
)

target_include_directories(rat_server PRIVATE
    ${SERVER_INCLUDE_DIR}
    ${COMMON_DIR}/include
    ${COMMON_DIR}/build/proto
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(rat_server
    PRIVATE
    ${COMMON_DIR}/build/librat_common.a
    ${COMMON_DIR}/build/proto/libproto.a
    ${Boost_LIBRARIES}
    ${OPENSSL_SSL_LIBRARY} 
    ${OPENSSL_CRYPTO_LIBRARY}  
    ${Protobuf_LIBRARY}
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(rat_server PRIVATE -Wall -Wextra)
else()
    target_compile_options(rat_server PRIVATE -O2 -Wall -Wextra)
endif()

set_target_properties(rat_server PROPERTIES POSITION_INDEPENDENT_CODE ON)