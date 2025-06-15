cmake_minimum_required(VERSION 3.10)
project(RatProjectTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(BUILD_SHARED_LIBS OFF)

set(TARGET_PLATFORM "" CACHE STRING "Target platform (ubuntu or centos7)")
if(NOT TARGET_PLATFORM)
    message(FATAL_ERROR "Vui lòng chỉ định -DTARGET_PLATFORM=centos7 hoặc ubuntu")
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Catch2 2.13.0 REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
if(TARGET_PLATFORM STREQUAL "centos7")
    set(Boost_USE_STATIC_RUNTIME ON)
    set(Boost_NO_SYSTEM_PATHS ON)
endif()
set(BOOST_ROOT /usr/local)
set(BOOST_LIBRARYDIR /usr/local/lib)
find_package(Boost 1.83 REQUIRED COMPONENTS system thread)

set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_ROOT_DIR /usr/local/openssl-3.0.14)
set(OPENSSL_INCLUDE_DIR /usr/local/openssl-3.0.14/include)
find_package(OpenSSL REQUIRED)

set(Protobuf_ROOT /usr/local/protobuf-static)
set(Protobuf_PROTOC_EXECUTABLE /usr/local/protobuf-static/bin/protoc)
set(Protobuf_INCLUDE_DIR /usr/local/protobuf-static/include)
set(Protobuf_LIBRARY /usr/local/protobuf-static/lib/libprotobuf.a)
find_package(Protobuf REQUIRED)

set(COMMON_INCLUDE_DIR /home/quang/rat_project/common/include)
# set(CLIENT_INCLUDE_DIR /home/quang/rat_project/client/include)
# set(SERVER_INCLUDE_DIR /home/quang/rat_project/server/include)
set(PROTO_BUILD_DIR /home/quang/rat_project/common/proto/build_${TARGET_PLATFORM})
set(COMMON_LIB_DIR /home/quang/rat_project/common/build_${TARGET_PLATFORM})

if(NOT EXISTS "${PROTO_BUILD_DIR}/libproto.a")
    message(FATAL_ERROR "Proto lib not found at ${PROTO_BUILD_DIR}/libproto.a")
endif()
if(NOT EXISTS "${COMMON_LIB_DIR}/librat_common.a")
    message(FATAL_ERROR "Common lib not found at ${COMMON_LIB_DIR}/librat_common.a")
endif()

include_directories(
    ${COMMON_INCLUDE_DIR}
    # ${CLIENT_INCLUDE_DIR}
    # ${SERVER_INCLUDE_DIR}
    ${PROTO_BUILD_DIR}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
)
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "Thư mục include: ${dir}")
endforeach()

set(TEST_SOURCES
    main_test.cpp
    test_NetworkManager.cpp
)

add_executable(rat_tests ${TEST_SOURCES})

# target_compile_definitions(rat_tests PRIVATE UNIT_TESTING)

target_link_libraries(rat_tests PRIVATE
    ${COMMON_LIB_DIR}/librat_common.a
    ${PROTO_BUILD_DIR}/libproto.a
    Boost::system
    Boost::thread
    OpenSSL::SSL
    OpenSSL::Crypto
    ${Protobuf_LIBRARIES}
    Catch2::Catch2
    -pthread
    -ldl
    -static-libstdc++
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(rat_tests PRIVATE -Wall -Wextra)
else()
    target_compile_options(rat_tests PRIVATE -O2 -Wall -Wextra)
endif()

include(CTest)
include(Catch)
catch_discover_tests(rat_tests)