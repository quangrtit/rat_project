cmake_minimum_required(VERSION 3.28)
project(RATCommon LANGUAGES CXX)

set(TARGET_PLATFORM "" CACHE STRING "Target platform (ubuntu or centos7)")
if(NOT TARGET_PLATFORM)
    message(FATAL_ERROR "Please specify -DTARGET_PLATFORM=centos7 or ubuntu")
endif()

set(Boost_USE_STATIC_LIBS ON)

if(TARGET_PLATFORM STREQUAL "centos7")
    set(Boost_USE_STATIC_RUNTIME ON)
    set(Boost_NO_SYSTEM_PATHS ON)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

set(BOOST_ROOT /usr/local)
set(OPENSSL_ROOT_DIR /usr/local/openssl-3.0.14)
set(Protobuf_ROOT /usr/local/protobuf-static)

set(PROTO_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto/build_${TARGET_PLATFORM}")


if(NOT EXISTS "${PROTO_BUILD_DIR}/libproto.a")
    message(FATAL_ERROR "Proto build directory ${PROTO_BUILD_DIR} not found or missing libproto.a")
endif()

find_package(Boost 1.83 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(Protobuf REQUIRED)

add_compile_options(-fvisibility=hidden)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROTO_BUILD_DIR}
)



set(COMMON_SOURCES
    src/NetworkManager.cpp
)

add_library(rat_common STATIC ${COMMON_SOURCES})

target_link_libraries(rat_common PUBLIC
    ${PROTO_BUILD_DIR}/libproto.a
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    ${Protobuf_LIBRARIES}
    -pthread
    -ldl
)

find_package(Catch2 2.13.0 REQUIRED)
if(Catch2_FOUND)
    message(STATUS "Catch2 found")
    enable_testing()

    set(TEST_SOURCES
        tests/test_network_manager.cpp
    )

    add_executable(rat_tests ${TEST_SOURCES})
    target_include_directories(rat_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PROTO_BUILD_DIR}
        ${Protobuf_INCLUDE_DIRS}
    )
    target_link_libraries(rat_tests PRIVATE
        rat_common
        Catch2::Catch2
        Boost::system
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
        ${Protobuf_LIBRARIES}
        -pthread
        -ldl
    )
    target_compile_options(rat_tests PRIVATE -fno-common)
    add_test(NAME RATTests COMMAND rat_tests)
else()
    message(FATAL_ERROR "Catch2 not found. Please install Catch2.")
endif()

set(RAT_COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE PATH "Include directory for RATCommon")
