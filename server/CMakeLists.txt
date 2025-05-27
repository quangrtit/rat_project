cmake_minimum_required(VERSION 3.28)
project(RATServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.83 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

set(COMMON_INCLUDE_DIR /home/quang/rat_project/common/include)
include_directories(${COMMON_INCLUDE_DIR})

set(SERVER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${SERVER_INCLUDE_DIR})

set(SERVER_SOURCES
    src/main.cpp
    src/Server.cpp
)

add_executable(rat_server ${SERVER_SOURCES})

target_link_libraries(rat_server
    PRIVATE
    /home/quang/rat_project/common/build/librat_common.a
    Boost::system
    Boost::thread
    OpenSSL::Crypto
)