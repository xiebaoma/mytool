/**
 * @file main.cpp
 * @brief SPDB SDK文件客户端工具 - 主程序入口
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 * 
 * 这是一个基于SPDB SDK的文件系统客户端工具，提供类似Linux命令行的文件操作接口。
 * 支持的功能包括：目录浏览、文件信息查看、文件内容显示等。
 */

#include <iostream>
#include <memory>
#include <exception>
#include <string>

#include "file_client.h"
#include "spdb_sdk_filesystem.h"

/**
 * @brief 程序主入口点
 * 
 * 初始化SPDB SDK文件系统，创建文件客户端并启动交互式界面。
 * 
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return int 程序退出码，0表示成功，1表示失败
 * 
 * @note 程序支持一个可选的命令行参数来指定根目录路径
 * @example 
 *   ./ops_tools /mysql/data    # 指定根目录
 *   ./ops_tools                # 使用默认根目录 /mysql/data
 */
int main(int argc, char* argv[]) {
    // 设置默认根目录
    const std::string DEFAULT_ROOT_DIRECTORY = "/mysql/data";
    
    try {
        std::string root_directory = DEFAULT_ROOT_DIRECTORY;

        // 解析命令行参数
        if (argc > 1) {
            root_directory = argv[1];
            
            // 简单的参数验证
            if (root_directory.empty()) {
                std::cerr << "Error: Root directory cannot be empty" << std::endl;
                return 1;
            }
        }

        // 显示启动信息
        std::cout << "Starting SPDB SDK File Client..." << std::endl;
        std::cout << "Root directory: " << root_directory << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        // 创建 SPDB_SDK 文件系统适配器
        auto filesystem = std::make_unique<file_client::SPDB_SDKFileSystem>(root_directory);

        // 创建文件客户端
        file_client::FileClient client(std::move(filesystem));

        // 运行交互式客户端
        client.run_interactive();

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "ops_tools")
                  << " [root_directory]" << std::endl;
        std::cerr << "Example: " << (argc > 0 ? argv[0] : "ops_tools") 
                  << " /mysql/data" << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Fatal Error: Unknown exception occurred" << std::endl;
        return 1;
    }

    return 0;
}

/*
int main() {
    SDK initialize("/etc/spdb/sdk_default_config.toml");

    char s[] = "/mysql/data/#ib_redo1";

    int fd = SDK_FILE open(s,  O_RDWR | O_CREAT);
    void *buf = malloc(4 * 1024*1024);
    memset(buf, 0, 4 * 1024*1024);
    SDK_FILE pwrite(fd, buf, 4 * 1024*1024, 0);

    SDK_FILE close(fd);
    sleep(1000000);
    free(buf);
    return 0;
}
*/
