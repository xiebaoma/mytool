/**
 * xiebaoma
 * 2025-08-22
 */

#include "file_client.h"
#include "linux_filesystem.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace file_client {

FileClient::FileClient(std::unique_ptr<FileSystemInterface> fs) 
    : filesystem_(std::move(fs)) {
}

CommandResult FileClient::execute_command(const std::string& command_line) {
    if (command_line.empty()) {
        return CommandResult(true, "");
    }
    
    auto tokens = parse_command(command_line);
    if (tokens.empty()) {
        return CommandResult(true, "");
    }
    
    const std::string& cmd = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    
    if (cmd == "ls") {
        return cmd_ls(args);
    } else if (cmd == "file") {
        return cmd_file(args);
    } else if (cmd == "stat") {
        return cmd_stat(args);
    } else if (cmd == "du") {
        return cmd_du(args);
    } else if (cmd == "cat") {
        return cmd_cat(args);
    } else if (cmd == "cd") {
        return cmd_cd(args);
    } else if (cmd == "pwd") {
        return cmd_pwd(args);
    } else if (cmd == "help" || cmd == "?") {
        return cmd_help(args);
    } else if (cmd == "exit" || cmd == "quit") {
        return CommandResult(false, "exit");
    } else {
        return CommandResult(false, "Unknown command: " + cmd + ", use 'help' for available commands");
    }
}

CommandResult FileClient::cmd_ls(const std::vector<std::string>& args) {
    bool long_format = false;
    std::string target_path = ".";
    
    // 解析参数
    for (const auto& arg : args) {
        if (arg == "-l") {
            long_format = true;
        } else if (arg[0] != '-') {
            target_path = arg;
        }
    }
    
    try {
        std::string resolved_path = filesystem_->resolve_path(target_path);
        
        if (!filesystem_->exists(resolved_path)) {
            return CommandResult(false, "Path does not exist: " + target_path);
        }
        
        if (!filesystem_->is_directory(resolved_path)) {
            // 如果是文件，显示文件信息
            FileInfo info = filesystem_->get_file_info(resolved_path);
            std::ostringstream oss;
            if (long_format) {
                oss << info.permissions_str << " "
                    << std::setw(10) << info.size << " "
                    << FileSystemInterface::format_time(info.mtime) << " "
                    << info.name;
            } else {
                oss << info.name;
            }
            return CommandResult(true, oss.str());
        }
        
        auto files = filesystem_->list_directory(resolved_path);
        if (files.empty()) {
            return CommandResult(true, "Directory is empty");
        }
        
        std::ostringstream result;
        for (const auto& file : files) {
            if (long_format) {
                result << file.permissions_str << " "
                       << std::setw(10) << file.size << " "
                       << FileSystemInterface::format_time(file.mtime) << " "
                       << file.name << "\n";
            } else {
                result << file.name << "  ";
            }
        }
        
        if (!long_format) {
            result << "\n";
        }
        
        return CommandResult(true, result.str());
        
    } catch (const std::exception& e) {
        return CommandResult(false, "Error: " + std::string(e.what()));
    }
}

CommandResult FileClient::cmd_file(const std::vector<std::string>& args) {
    if (args.empty()) {
        return CommandResult(false, "Usage: file <文件名>");
    }
    
    const std::string& filename = args[0];
    
    try {
        std::string resolved_path = filesystem_->resolve_path(filename);
        
        if (!filesystem_->exists(resolved_path)) {
            return CommandResult(false, "File does not exist: " + filename);
        }
        
        FileInfo info = filesystem_->get_file_info(resolved_path);
        std::ostringstream result;
        
        result << filename << ": " << FileSystemInterface::get_file_type_string(info.type);
        
        if (info.type == FileType::REGULAR_FILE) {
            // 尝试判断文件内容类型
            try {
                std::string content = filesystem_->read_file_content(resolved_path, 1024);
                if (is_text_file(content)) {
                    result << ", text file";
                } else {
                    result << ", binary file";
                }
                
                // 根据扩展名添加更多信息
                std::string mime_type = get_file_mime_type(filename);
                if (!mime_type.empty()) {
                    result << " (" << mime_type << ")";
                }
            } catch (...) {
                result << ", cannot read content";
            }
        }
        
        return CommandResult(true, result.str());
        
    } catch (const std::exception& e) {
        return CommandResult(false, "Error: " + std::string(e.what()));
    }
}

CommandResult FileClient::cmd_stat(const std::vector<std::string>& args) {
    if (args.empty()) {
        return CommandResult(false, "Usage: stat <文件名>");
    }
    
    const std::string& filename = args[0];
    
    try {
        std::string resolved_path = filesystem_->resolve_path(filename);
        
        if (!filesystem_->exists(resolved_path)) {
            return CommandResult(false, "File does not exist: " + filename);
        }
        
        FileInfo info = filesystem_->get_file_info(resolved_path);
        std::ostringstream result;
        
        result << "File: " << filename << "\n"
               << "Type: " << FileSystemInterface::get_file_type_string(info.type) << "\n"
               << "Size: " << info.size << " bytes\n"
               << "Permissions: " << info.permissions_str << " (0" 
               << std::oct << (info.mode & 0777) << std::dec << ")\n"
               << "Modified: " << FileSystemInterface::format_time(info.mtime) << "\n"
               << "Accessed: " << FileSystemInterface::format_time(info.atime) << "\n"
               << "Created: " << FileSystemInterface::format_time(info.ctime);
        
        return CommandResult(true, result.str());
        
    } catch (const std::exception& e) {
        return CommandResult(false, "Error: " + std::string(e.what()));
    }
}

CommandResult FileClient::cmd_du(const std::vector<std::string>& args) {
    bool human_readable = false;
    std::string target_path = ".";
    
    // 解析参数
    for (const auto& arg : args) {
        if (arg == "-h") {
            human_readable = true;
        } else if (arg[0] != '-') {
            target_path = arg;
        }
    }
    
    try {
        std::string resolved_path = filesystem_->resolve_path(target_path);
        
        if (!filesystem_->exists(resolved_path)) {
            return CommandResult(false, "Path does not exist: " + target_path);
        }
        
        size_t size = filesystem_->get_directory_size(resolved_path, true);
        std::string size_str = FileSystemInterface::format_file_size(size, human_readable);
        
        return CommandResult(true, size_str + "\t" + target_path);
        
    } catch (const std::exception& e) {
        return CommandResult(false, "Error: " + std::string(e.what()));
    }
}

CommandResult FileClient::cmd_cat(const std::vector<std::string>& args) {
    if (args.empty()) {
        return CommandResult(false, "Usage: cat <文件名>");
    }
    
    const std::string& filename = args[0];
    
    try {
        std::string resolved_path = filesystem_->resolve_path(filename);
        
        if (!filesystem_->exists(resolved_path)) {
            return CommandResult(false, "File does not exist: " + filename);
        }
        
        if (filesystem_->is_directory(resolved_path)) {
            return CommandResult(false, filename + " is a directory, cannot display content");
        }
        
        // 限制最大读取大小为1MB，避免内存问题
        std::string content = filesystem_->read_file_content(resolved_path, 1024 * 1024);
        
        if (content.empty()) {
            return CommandResult(true, "File is empty");
        }
        
        // 检查是否为二进制文件
        if (!is_text_file(content)) {
            return CommandResult(false, filename + " is a binary file, cannot display");
        }
        
        return CommandResult(true, content);
        
    } catch (const std::exception& e) {
        return CommandResult(false, "Error: " + std::string(e.what()));
    }
}

CommandResult FileClient::cmd_cd(const std::vector<std::string>& args) {
    std::string target_path = "/";
    
    if (!args.empty()) {
        target_path = args[0];
    }
    
    // 检查是否试图访问根目录以上
    LinuxFileSystem* linux_fs = dynamic_cast<LinuxFileSystem*>(filesystem_.get());
    if (linux_fs && linux_fs->is_trying_to_escape_root(target_path)) {
        return CommandResult(false, "Access denied: Cannot navigate above the root directory (" + target_path + ").\nCurrent root directory restricts access to its subdirectories only.");
    }
    
    if (filesystem_->change_directory(target_path)) {
        return CommandResult(true, "");
    } else {
        return CommandResult(false, "Cannot change to directory: " + target_path);
    }
}

CommandResult FileClient::cmd_pwd(const std::vector<std::string>&) {
    return CommandResult(true, filesystem_->get_current_directory());
}

CommandResult FileClient::cmd_help(const std::vector<std::string>&) {
    std::ostringstream help;
    help << "File Client Tool - Available Commands:\n\n"
         << "Directory Operations:\n"
         << "  ls [path]          List directory contents\n"
         << "  ls -l [path]       List detailed directory contents (permissions, size, time)\n"
         << "  cd [path]          Change directory\n"
         << "  pwd                Show current directory\n\n"
         << "File Information:\n"
         << "  file <filename>    Show file type\n"
         << "  stat <filename>    Show detailed file information\n"
         << "  du [path]          Show file/directory size (bytes)\n"
         << "  du -h [path]       Show human-readable size (KB/MB/GB)\n\n"
         << "File Content:\n"
         << "  cat <filename>     Display file content\n\n"
         << "Other:\n"
         << "  help               Show this help message\n"
         << "  exit/quit          Exit the program\n\n"
         << "Note: Access is restricted to the specified root directory";
    
    return CommandResult(true, help.str());
}

void FileClient::run_interactive() {
    // 获取LinuxFileSystem来显示真实根目录
    LinuxFileSystem* linux_fs = dynamic_cast<LinuxFileSystem*>(filesystem_.get());
    if (linux_fs) {
        std::cout << "File Client Tool started (Root directory: " << linux_fs->get_real_system_path() << ")\n";
    } else {
        std::cout << "File Client Tool started\n";
    }
    std::cout << "Type 'help' for available commands, 'exit' to quit\n\n";
    
    std::string input;
    while (true) {
        std::cout << get_prompt();
        std::getline(std::cin, input);
        
        auto result = execute_command(input);
        
        if (!result.success && result.message == "exit") {
            break;
        }
        
        if (!result.message.empty()) {
            std::cout << result.message << std::endl;
        }
        
        if (!result.success && result.message != "exit") {
            // Show error but continue running
        }
        
        std::cout << std::endl;
    }
    
    std::cout << "Goodbye!" << std::endl;
}

// 私有工具方法实现

std::vector<std::string> FileClient::parse_command(const std::string& command_line) {
    std::vector<std::string> tokens;
    std::istringstream iss(command_line);
    std::string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string FileClient::get_file_mime_type(const std::string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "";
    }
    
    std::string ext = filename.substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // 简单的MIME类型映射
    if (ext == "txt") return "text/plain";
    if (ext == "cpp" || ext == "cc" || ext == "c") return "text/x-c++src";
    if (ext == "h" || ext == "hpp") return "text/x-c++hdr";
    if (ext == "py") return "text/x-python";
    if (ext == "js") return "text/javascript";
    if (ext == "html") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "json") return "application/json";
    if (ext == "xml") return "application/xml";
    if (ext == "pdf") return "application/pdf";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "gif") return "image/gif";
    if (ext == "zip") return "application/zip";
    if (ext == "tar") return "application/x-tar";
    if (ext == "gz") return "application/gzip";
    
    return "";
}

bool FileClient::is_text_file(const std::string& content) {
    if (content.empty()) {
        return true;
    }
    
    // 简单的文本文件检测：检查是否包含过多的非可打印字符
    size_t non_printable = 0;
    size_t total = std::min(content.size(), size_t(512)); // 只检查前512字节
    
    for (size_t i = 0; i < total; ++i) {
        unsigned char c = content[i];
        if (c == 0) {
            // NULL字符通常表示二进制文件
            return false;
        }
        if (c < 32 && c != '\t' && c != '\n' && c != '\r') {
            non_printable++;
        }
    }
    
    // 如果非可打印字符超过30%，认为是二进制文件
    return (non_printable * 100 / total) < 30;
}

std::string FileClient::get_prompt() {
    LinuxFileSystem* linux_fs = dynamic_cast<LinuxFileSystem*>(filesystem_.get());
    if (linux_fs) {
        return "[" + linux_fs->get_real_system_path() + "] " + 
               filesystem_->get_current_directory() + " $ ";
    } else {
        return filesystem_->get_current_directory() + " $ ";
    }
}

} // namespace file_client
