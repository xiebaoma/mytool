cmake_minimum_required(VERSION 3.28)
project(ops_tools)

set(CMAKE_CXX_STANDARD 20)

find_library(SPDB_SDK_LIBRARY NAMES spdb-sdk REQUIRED)

set(SOURCES
        main.cpp
        file_client.cpp
        filesystem_interface.cpp
        spdb_sdk_filesystem.cpp
)

set(HEADERS
        file_client.h
        filesystem_interface.h
        spdb_sdk_filesystem.h
)

add_executable(ops_tools ${SOURCES} ${HEADERS})

target_link_libraries(ops_tools ${SPDB_SDK_LIBRARY})
