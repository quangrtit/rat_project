cmake_minimum_required(VERSION 3.28)
project(RATClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

set(Boost_USE_STATIC_LIBS ON)
set(OPENSSL_USE_STATIC_LIBS TRUE)

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