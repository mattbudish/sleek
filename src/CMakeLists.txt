##
##                   _____  __             __
##                  / ___/ / /___   ___   / /__
##                  \__ \ / // _ \ / _ \ / //_/
##                 ___/ // //  __//  __// ,<
##                /____//_/ \___/ \___//_/|_|
##
##
## Sleek Build System
##
## Build C/C++ projects without build files.
##
## Licensed under the MIT License <http://opensource.org/licenses/MIT>.
## Copyright 2023 Matt Budish <mtbudish@gmail.com>

cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

find_package(nlohmann_json)
find_package(argparse)
find_package(LibArchive)
find_package(CURL)
find_package(FLEX REQUIRED)

if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        json 
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )

    FetchContent_MakeAvailable(json)
endif()

if(NOT argparse_FOUND)
    FetchContent_Declare(
        argparse
        URL https://github.com/p-ranav/argparse/archive/refs/tags/v2.9.tar.gz
    )

    FetchContent_MakeAvailable(argparse)
endif()

if(NOT LibArchive_FOUND)
    FetchContent_Declare(
        libarchive 
        URL https://github.com/libarchive/libarchive/archive/refs/tags/v3.6.2.tar.gz
    )

    FetchContent_MakeAvailable(libarchive)
endif()

if(NOT CURL_FOUND)
    FetchContent_Declare(
        curl 
        URL https://github.com/curl/curl/releases/download/curl-8_1_2/curl-8.1.2.tar.gz
    )

    FetchContent_Populate(curl)
    add_subdirectory(${curl_SOURCE_DIR} ${curl_BINARY_DIR})
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/codeModel/)
FLEX_TARGET(cppParser ${CMAKE_CURRENT_LIST_DIR}/codeModel/cppParser.l ${CMAKE_CURRENT_BINARY_DIR}/codeModel/lex.yy.cpp)

file(GLOB_RECURSE SLEEK_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(sleek ${SLEEK_SOURCES} ${FLEX_cppParser_OUTPUTS})

target_include_directories(sleek PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/codeModel/)

target_link_libraries(
    sleek 
    PRIVATE 
    argparse::argparse 
    CURL::libcurl 
    LibArchive::LibArchive 
    nlohmann_json::nlohmann_json 
    pthread
    ${FLEX_LIBRARIES}
)

install(TARGETS sleek RUNTIME_DEPENDENCIES)