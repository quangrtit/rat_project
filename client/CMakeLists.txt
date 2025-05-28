cmake_minimum_required(VERSION 3.28)
project(RATClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Đồng bộ với RATCommon

# Tìm kiếm Boost
find_package(Boost 1.83 REQUIRED COMPONENTS system thread)

# Tìm kiếm Protobuf
find_package(Protobuf REQUIRED)

# Định nghĩa các thư mục include
set(CLIENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CLIENT_INCLUDE_DIR})

# Include thư mục từ RATCommon
set(COMMON_DIR ${CMAKE_SOURCE_DIR}/../common)
include_directories(${COMMON_DIR}/include ${COMMON_DIR}/build/proto) # Include cho Packet.pb.h

# Danh sách file nguồn
set(CLIENT_SOURCES
    src/main.cpp
    src/Client.cpp
)

# Tạo executable
add_executable(rat_client ${CLIENT_SOURCES})

# Link thư viện
target_link_libraries(rat_client
    PRIVATE
    ${COMMON_DIR}/build/librat_common.a # Link với rat_common
    ${COMMON_DIR}/build/proto/libproto.a
    Boost::system
    Boost::thread
    ${Protobuf_LIBRARIES}
)

# Tùy chọn tối ưu hóa (đồng bộ với RATCommon)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(rat_client PRIVATE -Wall -Wextra)
else()
    target_compile_options(rat_client PRIVATE -O2 -Wall -Wextra)
endif()