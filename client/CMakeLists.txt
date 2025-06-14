cmake_minimum_required(VERSION 3.28)
project(RATClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(BUILD_SHARED_LIBS OFF)

# === Chọn platform ===
set(TARGET_PLATFORM "" CACHE STRING "Target platform (ubuntu or centos7)")
if(NOT TARGET_PLATFORM)
    message(FATAL_ERROR "Please specify -DTARGET_PLATFORM=centos7 or ubuntu")
endif()

# === Boost ===
set(Boost_USE_STATIC_LIBS ON)
if(TARGET_PLATFORM STREQUAL "centos7")
    set(Boost_USE_STATIC_RUNTIME ON)
    set(Boost_NO_SYSTEM_PATHS ON)
endif()
set(BOOST_ROOT /usr/local)
set(BOOST_LIBRARYDIR /usr/local/lib)
find_package(Boost 1.83 REQUIRED COMPONENTS system thread)

# === OpenSSL ===
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_ROOT_DIR /usr/local/openssl-3.0.14)
set(OPENSSL_INCLUDE_DIR /usr/local/openssl-3.0.14/include)
find_package(OpenSSL REQUIRED)

# === Protobuf ===
set(Protobuf_ROOT /usr/local/protobuf-static)
set(Protobuf_PROTOC_EXECUTABLE /usr/local/protobuf-static/bin/protoc)
set(Protobuf_INCLUDE_DIR /usr/local/protobuf-static/include)
set(Protobuf_LIBRARY /usr/local/protobuf-static/lib/libprotobuf.a)
find_package(Protobuf REQUIRED)

# === Đường dẫn include chung ===
set(CLIENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMMON_DIR ${CMAKE_SOURCE_DIR}/../common)
set(PROTO_BUILD_DIR ${COMMON_DIR}/proto/build_${TARGET_PLATFORM})
set(COMMON_LIB_DIR ${COMMON_DIR}/build_${TARGET_PLATFORM})

# === Check tồn tại thư viện
if(NOT EXISTS "${PROTO_BUILD_DIR}/libproto.a")
    message(FATAL_ERROR "Proto lib not found at ${PROTO_BUILD_DIR}/libproto.a")
endif()
if(NOT EXISTS "${COMMON_LIB_DIR}/librat_common.a")
    message(FATAL_ERROR "Common lib not found at ${COMMON_LIB_DIR}/librat_common.a")
endif()

# === Source và executable
add_executable(rat_client
    src/main.cpp
    src/Client.cpp
    src/FileSender.cpp
    src/Utils.cpp
    src/ProcessSender.cpp
    src/ProcessUtils.cpp
    src/FileFolderSender.cpp
    src/FileFolderUtils.cpp
)

# === Include và link
target_include_directories(rat_client PRIVATE
    ${CLIENT_INCLUDE_DIR}
    ${COMMON_DIR}/include
    ${PROTO_BUILD_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(rat_client PRIVATE
    ${COMMON_LIB_DIR}/librat_common.a
    ${PROTO_BUILD_DIR}/libproto.a
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${Protobuf_LIBRARY}
    -static-libstdc++
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(rat_client PRIVATE -Wall -Wextra)
else()
    target_compile_options(rat_client PRIVATE -O2 -Wall -Wextra)
endif()

set_target_properties(rat_client PROPERTIES POSITION_INDEPENDENT_CODE ON)

# === Install
install(TARGETS rat_client DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/ca.crt DESTINATION etc/rat-client)

# === CPack cấu hình đóng gói đa nền tảng
set(CPACK_SET_DESTDIR ON)
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "rat-client")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "quang@example.com")
set(CPACK_PACKAGE_VENDOR "quang")
set(CPACK_PACKAGE_DESCRIPTION "RAT Client packaged with CPack")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://quang.com")

if(TARGET_PLATFORM STREQUAL "ubuntu")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
elseif(TARGET_PLATFORM STREQUAL "centos7")
    set(CPACK_GENERATOR "RPM")
    set(CPACK_RPM_PACKAGE_GROUP "Utilities")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
    set(CPACK_RPM_COMPRESSION_TYPE "xz")
    set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
else()
    message(FATAL_ERROR "Unknown TARGET_PLATFORM: ${TARGET_PLATFORM}")
endif()

include(CPack)
