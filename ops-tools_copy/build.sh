#!/bin/bash
set -e  # 出错就退出

# 创建并进入 build 目录
if [ ! -d "build" ]; then
  mkdir build
fi
cd build

# 运行 cmake 生成构建文件
cmake ..

# 编译
cmake --build .