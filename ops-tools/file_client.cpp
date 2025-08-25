/**
 * xiebaoma
 * 2025-08-25
 */

#include "file_client.h"
#include "spdb_sdk_filesystem.h"
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
    } else if (cmd == "hexdump") {
        return cmd_hexdump(args);
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
    SPDB_SDKFileSystem* spdb_sdk_fs = dynamic_cast<SPDB_SDKFileSystem*>(filesystem_.get());
    if (spdb_sdk_fs && spdb_sdk_fs->is_trying_to_escape_root(target_path)) {
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

CommandResult FileClient::cmd_hexdump(const std::vector<std::string>& args) {
    if (args.empty()) {
        return CommandResult(false, "Usage: hexdump [-offset N] [-len N] <filename>");
    }

    size_t file_offset = 0;
    size_t read_length = 0; // 0 means read to end
    std::string filename;

    // 解析参数
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-offset" && i + 1 < args.size()) {
            try {
                file_offset = std::stoull(args[i + 1]);
                i++; // 跳过下一个参数
            } catch (const std::exception&) {
                return CommandResult(false, "Invalid offset value: " + args[i + 1]);
            }
        } else if (args[i] == "-len" && i + 1 < args.size()) {
            try {
                read_length = std::stoull(args[i + 1]);
                i++; // 跳过下一个参数
            } catch (const std::exception&) {
                return CommandResult(false, "Invalid length value: " + args[i + 1]);
            }
        } else if (args[i][0] != '-') {
            filename = args[i];
        }
    }

    if (filename.empty()) {
        return CommandResult(false, "Usage: hexdump [-offset N] [-len N] <filename>");
    }

    try {
        std::string resolved_path = filesystem_->resolve_path(filename);

        if (!filesystem_->exists(resolved_path)) {
            return CommandResult(false, "File does not exist: " + filename);
        }

        if (filesystem_->is_directory(resolved_path)) {
            return CommandResult(false, filename + " is a directory, cannot hexdump");
        }

        // 读取文件内容，使用指定的偏移和长度
        std::string content;
        if (file_offset > 0 || read_length > 0) {
            // 限制最大读取长度避免内存问题
            size_t max_read_length = read_length;
            if (max_read_length == 0 || max_read_length > 1024 * 1024) {
                max_read_length = 1024 * 1024;
            }
            content = filesystem_->read_file_content_at_offset(resolved_path, file_offset, max_read_length);
        } else {
            // 使用原有方法，限制最大1MB
            content = filesystem_->read_file_content(resolved_path, 1024 * 1024);
        }

        if (content.empty()) {
            return CommandResult(true, "No data to display (file empty or offset beyond file size)");
        }

        std::ostringstream result;
        const size_t bytes_per_line = 8; // 每行显示8个字节，平衡可读性和宽度

        for (size_t line_offset = 0; line_offset < content.size(); line_offset += bytes_per_line) {
            // 显示实际的文件偏移地址（8位十六进制）
            size_t actual_offset = file_offset + line_offset;
            result << std::hex << std::setfill('0') << std::setw(8) << actual_offset << ": ";

            // 显示二进制字节
            std::string ascii_part;
            for (size_t i = 0; i < bytes_per_line; ++i) {
                if (line_offset + i < content.size()) {
                    unsigned char byte = static_cast<unsigned char>(content[line_offset + i]);
                    
                    // 将字节转换为8位二进制表示
                    for (int bit = 7; bit >= 0; --bit) {
                        result << ((byte >> bit) & 1);
                    }
                    result << " "; // 每个字节后添加空格

                    // 准备ASCII部分 - 可打印字符显示原字符，不可打印显示点
                    if (std::isprint(byte)) {
                        ascii_part += static_cast<char>(byte);
                    } else {
                        ascii_part += '.';
                    }
                } else {
                    // 填充空白，保持对齐 (8位二进制 + 1个空格)
                    result << "         ";
                    ascii_part += " ";
                }
            }

            // 显示ASCII字符
            result << " " << ascii_part << "\n";
        }

        return CommandResult(true, result.str());

    } catch (const std::exception& e) {
        return CommandResult(false, "Error: " + std::string(e.what()));
    }
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
         << "  cat <filename>     Display file content\n"
         << "  hexdump <filename> Display hexadecimal dump of file\n\n"
         << "Other:\n"
         << "  help               Show this help message\n"
         << "  exit/quit          Exit the program\n\n"
         << "Note: Access is restricted to the specified root directory";

    return CommandResult(true, help.str());
}

void FileClient::run_interactive() {

    SPDB_SDKFileSystem* spdb_sdk_fs = dynamic_cast<SPDB_SDKFileSystem*>(filesystem_.get());
    if (spdb_sdk_fs) {
        std::cout << "File Client Tool started (Root directory: " << spdb_sdk_fs->get_real_system_path() << ")\n";
    } else {
        std::cout << "File Client Tool started\n";
    }
    std::cout << "Type 'help' for available commands, 'exit' to quit\n\n";

    std::string input;
    while (true) {
        std::cout << get_prompt();     // 只显示提示符
        std::getline(std::cin, input); // 读取输入，但不打印

        auto result = execute_command(input);

        if (!result.success && result.message == "exit") {
            break;
        }

        // 只打印命令的执行结果
        if (!result.message.empty() && result.message != input) {
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
    if (content.empty()) return true;

    size_t non_printable = 0;
    size_t total = std::min(content.size(), size_t(512)); // 只检查前 512 字节

    size_t i = 0;
    while (i < total) {
        unsigned char c = static_cast<unsigned char>(content[i]);

        // NULL 字节直接判为二进制
        if (c == 0) return false;

        // ASCII 控制字符检测
        if (c < 32) {
            if (c != '\t' && c != '\n' && c != '\r') {
                non_printable++;
            }
            i++;
            continue;
        }

        // UTF-8 多字节检查
        size_t utf8_len = 0;
        if (c >= 0x80) {
            if ((c & 0xE0) == 0xC0) utf8_len = 2;
            else if ((c & 0xF0) == 0xE0) utf8_len = 3;
            else if ((c & 0xF8) == 0xF0) utf8_len = 4;
            else {
                // 非法 UTF-8 起始字节
                non_printable++;
                i++;
                continue;
            }

            // 检查后续字节是否合法
            bool valid = true;
            for (size_t j = 1; j < utf8_len && (i + j) < total; ++j) {
                unsigned char cc = static_cast<unsigned char>(content[i + j]);
                if ((cc & 0xC0) != 0x80) {
                    valid = false;
                    break;
                }
            }

            if (!valid) non_printable++;

            i += utf8_len;
            continue;
        }

        // 普通可打印 ASCII
        i++;
    }

    // 如果非可打印字符比例超过 30%，判为二进制
    return (non_printable * 100 / total) < 30;
}

std::string FileClient::get_prompt() {
    SPDB_SDKFileSystem* spdb_sdk_fs = dynamic_cast<SPDB_SDKFileSystem*>(filesystem_.get());
    if (spdb_sdk_fs) {
        return "[" + spdb_sdk_fs->get_real_system_path() + "] " +
               filesystem_->get_current_directory() + " $ ";
    } else {
        return filesystem_->get_current_directory() + " $ ";
    }
}

} // namespace file_client
