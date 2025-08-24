/**
 * xiebaoma
 * 2025-08-22
 */

#pragma once

#include "filesystem_interface.h"
#include <memory>
#include <string>
#include <vector>

namespace file_client {

// 命令结果结构
struct CommandResult {
    bool success;
    std::string message;
    
    CommandResult(bool s = true, const std::string& msg = "") 
        : success(s), message(msg) {}
};

// 文件客户端主类
class FileClient {
public:
    explicit FileClient(std::unique_ptr<FileSystemInterface> fs);
    ~FileClient() = default;

    // 主要的命令处理方法
    CommandResult execute_command(const std::string& command_line);
    
    // 具体命令实现
    CommandResult cmd_ls(const std::vector<std::string>& args);
    CommandResult cmd_file(const std::vector<std::string>& args);
    CommandResult cmd_stat(const std::vector<std::string>& args);
    CommandResult cmd_du(const std::vector<std::string>& args);
    CommandResult cmd_cat(const std::vector<std::string>& args);
    CommandResult cmd_cd(const std::vector<std::string>& args);
    CommandResult cmd_pwd(const std::vector<std::string>& args);
    CommandResult cmd_help(const std::vector<std::string>& args);
    
    // 交互式运行
    void run_interactive();
    
    // 获取提示符
    std::string get_prompt();

private:
    std::unique_ptr<FileSystemInterface> filesystem_;
    
    // 工具方法
    std::vector<std::string> parse_command(const std::string& command_line);
    void print_help();
    std::string get_file_mime_type(const std::string& filename);
    bool is_text_file(const std::string& content);
};

} // namespace file_client
