cmake_minimum_required(VERSION 3.28)
project(RATClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.83 REQUIRED COMPONENTS system thread)

set(COMMON_INCLUDE_DIR /home/quang/rat_project/common/include)
include_directories(${COMMON_INCLUDE_DIR})

set(CLIENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CLIENT_INCLUDE_DIR})

set(CLIENT_SOURCES
    src/main.cpp
    src/Client.cpp
)

add_executable(rat_client ${CLIENT_SOURCES})

target_link_libraries(rat_client
    PRIVATE
    /home/quang/rat_project/common/build/librat_common.a
    Boost::system
    Boost::thread
)