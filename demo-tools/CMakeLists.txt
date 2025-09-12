cmake_minimum_required(VERSION 3.10)
project(file_client_tool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 源文件列表
set(SOURCES
    main.cpp
    filesystem_interface.cpp
    linux_filesystem.cpp
    file_client.cpp
)

# 头文件列表
set(HEADERS
    filesystem_interface.h
    linux_filesystem.h
    file_client.h
    sdk.h
    file_io.h
    dir.h
)

# 创建可执行文件
add_executable(file_client ${SOURCES})

# 设置包含目录
target_include_directories(file_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接系统库
if(UNIX)
    target_link_libraries(file_client pthread)
endif()

# 可选：保留原来的SPDB SDK配置作为注释，以后需要时可以启用
# find_library(SPDB_SDK_LIBRARY NAMES spdb-sdk REQUIRED)
# target_link_libraries(file_client ${SPDB_SDK_LIBRARY})

# 安装规则（可选）
install(TARGETS file_client DESTINATION bin)
