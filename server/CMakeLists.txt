cmake_minimum_required(VERSION 3.28)
project(RATServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Đồng bộ với RATCommon

# Tìm kiếm Boost
find_package(Boost 1.83 REQUIRED COMPONENTS system thread)

# Tìm kiếm OpenSSL
find_package(OpenSSL REQUIRED)

# Tìm kiếm Protobuf
find_package(Protobuf REQUIRED)

# Định nghĩa các thư mục include
set(SERVER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${SERVER_INCLUDE_DIR})

# Include thư mục từ RATCommon
set(COMMON_DIR ${CMAKE_SOURCE_DIR}/../common)
include_directories(${COMMON_DIR}/include ${COMMON_DIR}/build/proto) # Include cho Packet.pb.h

# Danh sách file nguồn
set(SERVER_SOURCES
    src/main.cpp
    src/Server.cpp
)

# Tạo executable
add_executable(rat_server ${SERVER_SOURCES})

# Link thư viện
target_link_libraries(rat_server
    PRIVATE
    ${COMMON_DIR}/build/librat_common.a # Link với rat_common
    ${COMMON_DIR}/build/proto/libproto.a
    Boost::system
    Boost::thread
    OpenSSL::Crypto
    ${Protobuf_LIBRARIES}
)

# Tùy chọn tối ưu hóa (đồng bộ với RATCommon)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(rat_server PRIVATE -Wall -Wextra)
else()
    target_compile_options(rat_server PRIVATE -O2 -Wall -Wextra)
endif()