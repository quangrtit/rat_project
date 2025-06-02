cmake_minimum_required(VERSION 3.28)
project(RATCommon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost 1.83 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDE_DIR})

set(COMMON_SOURCES
    src/NetworkManager.cpp
)

add_library(rat_common STATIC ${COMMON_SOURCES})

add_subdirectory(proto)

target_include_directories(rat_common PUBLIC
    ${COMMON_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/build
)

target_link_libraries(rat_common
    PUBLIC
        Boost::system
        Boost::thread
        OpenSSL::SSL
        proto
)

find_package(Catch2 2.13.0 REQUIRED) 
if(Catch2_FOUND)
    enable_testing()
    set(TEST_SOURCES
        tests/test_network_manager.cpp
    )
    add_executable(rat_tests ${TEST_SOURCES})
    target_include_directories(rat_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/proto/build)
    target_link_libraries(rat_tests
        PRIVATE
            rat_common
            proto
            Catch2::Catch2
            Boost::system
            Boost::thread
            OpenSSL::SSL    
    )
    add_test(NAME RATTests COMMAND rat_tests)
else()
    message(FATAL_ERROR "Catch2 not found. Please install Catch2.")
endif()

set(RAT_COMMON_INCLUDE_DIR ${COMMON_INCLUDE_DIR})
