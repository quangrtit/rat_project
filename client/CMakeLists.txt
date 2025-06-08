cmake_minimum_required(VERSION 3.28)
project(RATClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 


find_package(Boost 1.83 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

find_package(Protobuf REQUIRED)


set(CLIENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CLIENT_INCLUDE_DIR})


set(COMMON_DIR ${CMAKE_SOURCE_DIR}/../common)
include_directories(${COMMON_DIR}/include ${COMMON_DIR}/build/proto)


set(CLIENT_SOURCES
    src/main.cpp
    src/Client.cpp
    src/FileSender.cpp
    src/Utils.cpp
    src/ProcessSender.cpp
    src/ProcessUtils.cpp
    src/FileFolderSender.cpp
    src/FileFolderUtils.cpp
)


add_executable(rat_client ${CLIENT_SOURCES})


target_link_libraries(rat_client
    PRIVATE
    ${COMMON_DIR}/build/librat_common.a
    ${COMMON_DIR}/build/proto/libproto.a
    Boost::system
    Boost::thread
    OpenSSL::SSL
    ${Protobuf_LIBRARIES}
)

# Tùy chọn tối ưu hóa (đồng bộ với RATCommon)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(rat_client PRIVATE -Wall -Wextra)
else()
    target_compile_options(rat_client PRIVATE -O2 -Wall -Wextra)
endif()