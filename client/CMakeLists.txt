cmake_minimum_required(VERSION 3.28)
project(RATClient LANGUAGES CXX)

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
find_package(OpenSSL REQUIRED)
if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found!")
endif()
message(STATUS "OpenSSL found at: ${OPENSSL_INCLUDE_DIR} - Libraries: ${OPENSSL_LIBRARIES}")

set(Protobuf_ROOT /usr/local/protobuf-static)
set(Protobuf_PROTOC_EXECUTABLE /usr/local/protobuf-static/bin/protoc)
set(Protobuf_INCLUDE_DIR /usr/local/protobuf-static/include)
set(Protobuf_LIBRARY /usr/local/protobuf-static/lib/libprotobuf.a)
find_package(Protobuf REQUIRED)
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf not found!")
endif()
message(STATUS "Protobuf found at: ${Protobuf_INCLUDE_DIRS}")

set(CLIENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMMON_DIR ${CMAKE_SOURCE_DIR}/../common)

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

target_include_directories(rat_client PRIVATE
    ${CLIENT_INCLUDE_DIR}
    ${COMMON_DIR}/include
    ${COMMON_DIR}/build/proto
    ${OPENSSL_INCLUDE_DIR}  # Thêm đường dẫn include của OpenSSL
    ${Boost_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(rat_client
    PRIVATE
    ${COMMON_DIR}/build/librat_common.a
    ${COMMON_DIR}/build/proto/libproto.a
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${Protobuf_LIBRARY}
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(rat_client PRIVATE -Wall -Wextra)
else()
    target_compile_options(rat_client PRIVATE -O2 -Wall -Wextra)
endif()

set_target_properties(rat_client PROPERTIES POSITION_INDEPENDENT_CODE ON)

install(TARGETS rat_client DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/ca.crt DESTINATION etc/rat-client)

set(CPACK_SET_DESTDIR ON)
include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "rat-client")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "quang@example.com")
set(CPACK_PACKAGE_VENDOR "quang")
set(CPACK_PACKAGE_DESCRIPTION "RAT Client đóng gói bằng CPack")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://quang.com")

set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

include(CPack)