/**
 * @file main.cpp
 * @author xiebaoma
 * @date 2025-08-22
 */


#include <iostream>
#include <memory>
#include <exception>
#include "file_client.h"
#include "linux_filesystem.h"

int main(int argc, char* argv[]) {
    try {
        // 默认根目录为test，可以通过命令行参数指定
        std::string root_directory = "test";
        
        if (argc > 1) {
            root_directory = argv[1];
        }
        
        // 创建Linux文件系统适配器
        auto filesystem = std::make_unique<file_client::LinuxFileSystem>(root_directory);
        // TO DO：创建 spdb_sdk文件系统适配器
        //auto filesystem = std::make_unique<file_client::SPDB_SDKFileSystem>(root_directory);
        
        // 创建文件客户端
        file_client::FileClient client(std::move(filesystem));
        
        // 运行交互式客户端
        client.run_interactive();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "file_client") 
                  << " [root_directory]" << std::endl;
        return 1;
    }
    
    return 0;
}