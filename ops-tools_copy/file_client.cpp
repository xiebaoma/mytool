/**
 * @file file_client.cpp
 * @brief File client class implementation
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 *
 * Implementation of FileClient class providing Unix-like command line interface
 * for file system operations.
 */

#include "file_client.h"
#include "spdb_sdk_filesystem.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <functional>

namespace file_client
{

    FileClient::FileClient(std::unique_ptr<FileSystemInterface> fs)
        : filesystem_(std::move(fs))
    {
    }

    CommandResult FileClient::execute_command(const std::string &command_line)
    {
        if (command_line.empty())
        {
            return CommandResult(true, "");
        }
        auto tokens = parse_command(command_line);
        if (tokens.empty())
        {
            return CommandResult(true, "");
        }
        const std::string &cmd = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        static const std::unordered_map<std::string, CommandResult (FileClient::*)(const std::vector<std::string> &)> command_map = {
            {"ls", &FileClient::cmd_ls},
            {"file", &FileClient::cmd_file},
            {"stat", &FileClient::cmd_stat},
            //{"du", &FileClient::cmd_du},
            {"cat", &FileClient::cmd_cat},
            {"cd", &FileClient::cmd_cd},
            {"pwd", &FileClient::cmd_pwd},
            {"hexdump", &FileClient::cmd_hexdump},
            {"help", &FileClient::cmd_help},
            {"?", &FileClient::cmd_help},
        };
        if (cmd == "exit" || cmd == "quit")
        {
            return CommandResult(false, "exit");
        }
        auto it = command_map.find(cmd);
        if (it != command_map.end())
        {
            return (this->*(it->second))(args); // 调用对应函数
        }
        return CommandResult(false, "Unknown command: " + cmd + ", use 'help' for available commands");
    }

    CommandResult FileClient::cmd_ls(const std::vector<std::string> &args)
    {
        bool long_format = false;
        std::string target_path = ".";

        // Parse arguments
        for (const auto &arg : args)
        {
            if (arg == "-l")
            {
                long_format = true;
            }
            else if (arg[0] != '-')
            {
                target_path = arg;
            }
        }

        try
        {
            std::string resolved_path = filesystem_->resolve_path(target_path);

            if (!filesystem_->exists(resolved_path))
            {
                return CommandResult(false, "Path does not exist: " + target_path);
            }

            if (!filesystem_->is_directory(resolved_path))
            {
                // If it's a file, display file information
                FileInfo info = filesystem_->get_file_info(resolved_path);
                std::ostringstream oss;

                // Note: SDK doesn't provide permission and time information
                if (long_format)
                {
                    oss << info.name << " "
                        << std::setw(10) << info.size << " " << "byte" << " "
                        << FileSystemInterface::get_file_type_string(info.type);
                }
                else
                {
                    oss << info.name;
                }
                return CommandResult(true, oss.str());
            }

            auto files = filesystem_->list_directory(resolved_path);
            if (files.empty())
            {
                return CommandResult(true, "Directory is empty");
            }

            std::ostringstream result;
            for (const auto &file : files)
            {
                if (long_format)
                {
                    result << file.name << " "
                           << std::setw(10) << file.size << " " << "byte" << " "
                           << FileSystemInterface::get_file_type_string(file.type)
                           << std::endl;
                }
                else
                {
                    result << file.name << "  ";
                }
            }

            if (!long_format)
            {
                result << "\n";
            }

            return CommandResult(true, result.str());
        }
        catch (const std::exception &e)
        {
            return CommandResult(false, "Error: " + std::string(e.what()));
        }
    }

    CommandResult FileClient::cmd_file(const std::vector<std::string> &args)
    {
        if (args.empty())
        {
            return CommandResult(false, "Usage: file <filename>");
        }

        const std::string &filename = args[0];

        try
        {
            std::string resolved_path = filesystem_->resolve_path(filename);

            if (!filesystem_->exists(resolved_path))
            {
                return CommandResult(false, "File does not exist: " + filename);
            }

            FileInfo info = filesystem_->get_file_info(resolved_path);
            std::ostringstream result;

            result << filename << ": " << FileSystemInterface::get_file_type_string(info.type);

            if (info.type == FileType::REGULAR_FILE)
            {
                // Try to determine file content type
                try
                {
                    std::string content = filesystem_->read_file_content(resolved_path, 1024);
                    if (is_text_file(content))
                    {
                        result << ", text file";
                    }
                    else
                    {
                        result << ", binary file";
                    }

                    // Add more information based on file extension
                    std::string mime_type = get_file_mime_type(filename);
                    if (!mime_type.empty())
                    {
                        result << " (" << mime_type << ")";
                    }
                }
                catch (...)
                {
                    result << ", cannot read content";
                }
            }

            return CommandResult(true, result.str());
        }
        catch (const std::exception &e)
        {
            return CommandResult(false, "Error: " + std::string(e.what()));
        }
    }

    CommandResult FileClient::cmd_stat(const std::vector<std::string> &args)
    {
        if (args.empty())
        {
            return CommandResult(false, "Usage: stat <filename>");
        }

        const std::string &filename = args[0];

        try
        {
            std::string resolved_path = filesystem_->resolve_path(filename);

            if (!filesystem_->exists(resolved_path))
            {
                return CommandResult(false, "File does not exist: " + filename);
            }

            if (filesystem_->is_directory(resolved_path))
            {
                return CommandResult(false, filename + " is a directory");
            }

            FileInfo info = filesystem_->get_file_info(resolved_path);
            std::ostringstream result;

            // Note: SDK doesn't provide permission and time information
            result << "File: " << filename << "\n"
                   << "Type: " << FileSystemInterface::get_file_type_string(info.type) << "\n"
                   << "Size: " << info.size << " bytes\n";
            //<< "Permissions: " << info.permissions_str << " (0"
            //<< std::oct << (info.mode & 0777) << std::dec << ")\n"
            //<< "Modified: " << FileSystemInterface::format_time(info.mtime) << "\n"
            //<< "Accessed: " << FileSystemInterface::format_time(info.atime) << "\n"
            //<< "Created: " << FileSystemInterface::format_time(info.ctime);

            return CommandResult(true, result.str());
        }
        catch (const std::exception &e)
        {
            return CommandResult(false, "Error: " + std::string(e.what()));
        }
    }

    CommandResult FileClient::cmd_du(const std::vector<std::string> &args)
    {
        bool human_readable = false;
        std::string target_path = ".";

        // Parse arguments
        for (const auto &arg : args)
        {
            if (arg == "-h")
            {
                human_readable = true;
            }
            else if (arg[0] != '-')
            {
                target_path = arg;
            }
        }

        try
        {
            std::string resolved_path = filesystem_->resolve_path(target_path);

            if (!filesystem_->exists(resolved_path))
            {
                return CommandResult(false, "Path does not exist: " + target_path);
            }

            uint64_t size = 0;
            if (filesystem_->is_directory(resolved_path))
            {
                size = filesystem_->get_directory_size(resolved_path, true);
            }
            else
            {
                size = filesystem_->get_file_size(resolved_path);
            }

            std::string size_str = FileSystemInterface::format_file_size(size, human_readable);

            return CommandResult(true, size_str + "\t" + target_path);
        }
        catch (const std::exception &e)
        {
            return CommandResult(false, "Error: " + std::string(e.what()));
        }
    }

    CommandResult FileClient::cmd_cat(const std::vector<std::string> &args)
    {
        if (args.empty())
        {
            return CommandResult(false, "Usage: cat <filename>");
        }

        const std::string &filename = args[0];

        try
        {
            std::string resolved_path = filesystem_->resolve_path(filename);

            if (!filesystem_->exists(resolved_path))
            {
                return CommandResult(false, "File does not exist: " + filename);
            }

            if (filesystem_->is_directory(resolved_path))
            {
                return CommandResult(false, filename + " is a directory, cannot display content");
            }

            // Limit maximum read size to 1MB to avoid memory issues
            std::string content = filesystem_->read_file_content(resolved_path, 1024 * 1024);

            if (content.empty())
            {
                return CommandResult(true, "File is empty");
            }

            // Check if it's a binary file
            if (!is_text_file(content))
            {
                return CommandResult(false, filename + " is a binary file, cannot display");
            }

            return CommandResult(true, content);
        }
        catch (const std::exception &e)
        {
            return CommandResult(false, "Error: " + std::string(e.what()));
        }
    }

    CommandResult FileClient::cmd_cd(const std::vector<std::string> &args)
    {
        std::string target_path = "/";

        if (!args.empty())
        {
            target_path = args[0];
        }

        // Check if trying to access above root directory
        SPDB_SDKFileSystem *spdb_sdk_fs = dynamic_cast<SPDB_SDKFileSystem *>(filesystem_.get());
        if (spdb_sdk_fs && spdb_sdk_fs->is_trying_to_escape_root(target_path))
        {
            return CommandResult(false, "Access denied: Cannot navigate above the root directory (" + target_path + ").\nCurrent root directory restricts access to its subdirectories only.");
        }

        if (filesystem_->change_directory(target_path))
        {
            return CommandResult(true, "");
        }
        else
        {
            return CommandResult(false, "Cannot change to directory: " + target_path);
        }
    }

    CommandResult FileClient::cmd_pwd(const std::vector<std::string> &)
    {
        return CommandResult(true, filesystem_->get_current_directory());
    }

    CommandResult FileClient::cmd_hexdump(const std::vector<std::string> &args)
    {
        if (args.empty())
        {
            return CommandResult(false, "Usage: hexdump [-offset N] [-len N] <filename>");
        }
        size_t file_offset = 0;
        size_t read_length = 0; // 0 means read to end
        std::string filename;
        // Parse arguments
        for (size_t i = 0; i < args.size(); ++i)
        {
            if (args[i] == "-offset" && i + 1 < args.size())
            {
                try
                {
                    file_offset = std::stoull(args[i + 1]);
                    i++; // Skip next parameter
                }
                catch (const std::exception &)
                {
                    return CommandResult(false, "Invalid offset value: " + args[i + 1]);
                }
            }
            else if (args[i] == "-len" && i + 1 < args.size())
            {
                try
                {
                    read_length = std::stoull(args[i + 1]);
                    i++; // Skip next parameter
                }
                catch (const std::exception &)
                {
                    return CommandResult(false, "Invalid length value: " + args[i + 1]);
                }
            }
            else if (args[i][0] != '-')
            {
                filename = args[i];
            }
        }
        if (filename.empty())
        {
            return CommandResult(false, "Usage: hexdump [-offset N] [-len N] <filename>");
        }
        try
        {
            std::string resolved_path = filesystem_->resolve_path(filename);
            if (!filesystem_->exists(resolved_path))
            {
                return CommandResult(false, "File does not exist: " + filename);
            }
            if (filesystem_->is_directory(resolved_path))
            {
                return CommandResult(false, filename + " is a directory, cannot hexdump");
            }
            // Read file content using specified offset and length
            std::string content;
            if (file_offset > 0 || read_length > 0)
            {
                // Limit maximum read length to avoid memory issues
                size_t max_read_length = read_length;
                if (max_read_length == 0 || max_read_length > 1024 * 1024)
                {
                    max_read_length = 1024 * 1024;
                }
                content = filesystem_->read_file_content_at_offset(resolved_path, file_offset, max_read_length);
            }
            else
            {
                // Use original method, limit to maximum 1MB
                content = filesystem_->read_file_content(resolved_path, 1024 * 1024);
            }
            if (content.empty())
            {
                return CommandResult(true, "No data to display (file empty or offset beyond file size)");
            }

            std::ostringstream result;
            const size_t bytes_per_line = 8; // Display 8 bytes per line, balancing readability and width

            for (size_t line_offset = 0; line_offset < content.size(); line_offset += bytes_per_line)
            {
                // Display actual file offset address (8-digit hexadecimal)
                size_t actual_offset = file_offset + line_offset;
                result << std::hex << std::setfill('0') << std::setw(8) << actual_offset << ": ";

                // Display binary bytes
                std::string ascii_part;
                for (size_t i = 0; i < bytes_per_line; ++i)
                {
                    if (line_offset + i < content.size())
                    {
                        unsigned char byte = static_cast<unsigned char>(content[line_offset + i]);

                        // Convert byte to 8-bit binary representation
                        for (int bit = 7; bit >= 0; --bit)
                        {
                            result << ((byte >> bit) & 1);
                        }
                        result << " "; // Add space after each byte

                        // Prepare ASCII part - printable characters show original, non-printable show dots
                        if (std::isprint(byte))
                        {
                            ascii_part += static_cast<char>(byte);
                        }
                        else
                        {
                            ascii_part += '.';
                        }
                    }
                    else
                    {
                        // Fill with blanks to maintain alignment (8-bit binary + 1 space)
                        result << "         ";
                        ascii_part += " ";
                    }
                }

                // Display ASCII characters
                result << " " << ascii_part << "\n";
            }

            return CommandResult(true, result.str());
        }
        catch (const std::exception &e)
        {
            return CommandResult(false, "Error: " + std::string(e.what()));
        }
    }

    CommandResult FileClient::cmd_help(const std::vector<std::string> &)
    {
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
             << "File Content:\n"
             << "  cat <filename>     Display file content\n"
             << "  hexdump <filename> Display hexadecimal dump of file\n\n"
             << "    hexdump -len -offset <filename>"
             << "Other:\n"
             << "  help               Show this help message\n"
             << "  exit/quit          Exit the program\n\n"
             << "Note: Access is restricted to the specified root directory";

        return CommandResult(true, help.str());
    }

    void FileClient::run_interactive()
    {
        SPDB_SDKFileSystem *spdb_sdk_fs = dynamic_cast<SPDB_SDKFileSystem *>(filesystem_.get());
        if (spdb_sdk_fs)
        {
            std::cout << "File Client Tool started (Root directory: " << spdb_sdk_fs->get_real_system_path() << ")\n";
        }
        else
        {
            std::cout << "File Client Tool started\n";
        }
        std::cout << "Type 'help' for available commands, 'exit' to quit\n\n";
        std::string input;
        while (true)
        {
            std::cout << get_prompt(); // 打印提示符
            if (!std::getline(std::cin, input))
            {
                break; // EOF or input error
            }
            auto result = execute_command(input);
            if (!result.success)
            {
                std::cerr << "Error: " << result.message << std::endl;
            }
            if (result.message == "exit")
            {
                break; // Exit command
            }
            if (!result.message.empty())
            {
                std::cout << result.message << std::endl;
            }
            std::cout << std::endl;
        }
        std::cout << "Goodbye!" << std::endl;
    }

    // Private utility method implementations

    std::vector<std::string> FileClient::parse_command(const std::string &command_line)
    {
        std::vector<std::string> tokens;
        std::istringstream iss(command_line);
        std::string token;

        while (iss >> token)
        {
            tokens.push_back(token);
        }

        return tokens;
    }

    std::string FileClient::get_file_mime_type(const std::string &filename)
    {
        static const std::unordered_map<std::string, std::string> mime_map = {
            {"txt", "text/plain"},
            {"cpp", "text/x-c++src"},
            {"cc", "text/x-c++src"},
            {"c", "text/x-c++src"},
            {"h", "text/x-c++hdr"},
            {"hpp", "text/x-c++hdr"},
            {"py", "text/x-python"},
            {"js", "text/javascript"},
            {"html", "text/html"},
            {"css", "text/css"},
            {"json", "application/json"},
            {"xml", "application/xml"},
            {"pdf", "application/pdf"},
            {"jpg", "image/jpeg"},
            {"jpeg", "image/jpeg"},
            {"png", "image/png"},
            {"gif", "image/gif"},
            {"zip", "application/zip"},
            {"tar", "application/x-tar"},
            {"gz", "application/gzip"},
        };

        size_t dot_pos = filename.find_last_of('.');
        if (dot_pos == std::string::npos)
        {
            return "";
        }

        std::string ext = filename.substr(dot_pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        auto it = mime_map.find(ext);
        return (it != mime_map.end()) ? it->second : "";
    }

    bool FileClient::is_text_file(const std::string &content)
    {
        if (content.empty())
            return true;

        size_t non_printable = 0;
        size_t total = std::min(content.size(), size_t(512)); // Only check first 512 bytes

        size_t i = 0;
        while (i < total)
        {
            unsigned char c = static_cast<unsigned char>(content[i]);

            // NULL bytes are directly considered binary
            if (c == 0)
                return false;

            // ASCII control character detection
            if (c < 32)
            {
                if (c != '\t' && c != '\n' && c != '\r')
                {
                    non_printable++;
                }
                i++;
                continue;
            }

            // UTF-8 multi-byte check
            size_t utf8_len = 0;
            if (c >= 0x80)
            {
                if ((c & 0xE0) == 0xC0)
                    utf8_len = 2;
                else if ((c & 0xF0) == 0xE0)
                    utf8_len = 3;
                else if ((c & 0xF8) == 0xF0)
                    utf8_len = 4;
                else
                {
                    // Invalid UTF-8 starting byte
                    non_printable++;
                    i++;
                    continue;
                }

                // Check if subsequent bytes are valid
                bool valid = true;
                for (size_t j = 1; j < utf8_len && (i + j) < total; ++j)
                {
                    unsigned char cc = static_cast<unsigned char>(content[i + j]);
                    if ((cc & 0xC0) != 0x80)
                    {
                        valid = false;
                        break;
                    }
                }

                if (!valid)
                    non_printable++;

                i += utf8_len;
                continue;
            }

            // Regular printable ASCII
            i++;
        }

        // If non-printable character ratio exceeds 30%, consider it binary
        return (non_printable * 100 / total) < 30;
    }

    std::string FileClient::get_prompt()
    {
        SPDB_SDKFileSystem *spdb_sdk_fs = dynamic_cast<SPDB_SDKFileSystem *>(filesystem_.get());
        if (spdb_sdk_fs)
        {
            return "[" + spdb_sdk_fs->get_real_system_path() + "] " +
                   filesystem_->get_current_directory() + " $ ";
        }
        else
        {
            return filesystem_->get_current_directory() + " $ ";
        }
    }

} // namespace file_client
