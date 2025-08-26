/**
 * @file spdb_sdk_filesystem.cpp
 * @brief SPDB SDK file system implementation
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 *
 * Implementation of SPDB SDK file system class providing secure access
 * to files and directories within a specified root directory.
 */

#include "spdb_sdk_filesystem.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <fcntl.h>

#include <spdb-sdk/sdk/sdk.h>
#include <spdb-sdk/file/file_io.h>
#include <spdb-sdk/file/dir.h>

namespace file_client
{

    SPDB_SDKFileSystem::SPDB_SDKFileSystem(const std::string &root_path)
        : current_path_("/")
    {
        spdb::sdk::initialize("/etc/spdb/sdk_default_config.toml");

        // Convert to absolute path
        if (char *abs_path = spdb::sdk::file::realpath(root_path.c_str(), nullptr))
        {
            root_path_ = std::string(abs_path);
            free(abs_path);
        }
        else
        {
            // If path doesn't exist, try to create it
            if (spdb::sdk::file::mkdir(root_path.c_str(), 0755) == 0)
            {
                abs_path = spdb::sdk::file::realpath(root_path.c_str(), nullptr);
                if (abs_path)
                {
                    root_path_ = std::string(abs_path);
                    free(abs_path);
                }
                else
                {
                    root_path_ = root_path; // fallback
                }
            }
            else
            {
                root_path_ = root_path; // fallback
            }
        }

        // Ensure root path ends with /
        if (root_path_.back() != '/')
        {
            root_path_ += '/';
        }

        test_initialize();
    }

    void SPDB_SDKFileSystem::test_initialize()
    {
        char redo1[] = "/mysql/data/#ib_redo1";
        char redo2[] = "/mysql/data/#ib_redo2";
        char redo3[] = "/mysql/data/#ib_redo3";
        char redo4[] = "/mysql/data/#ib_redo4";

        // Open files
        int fd_redo1 = spdb::sdk::file::open(redo1, O_RDWR | O_CREAT, 0644);
        int fd_redo2 = spdb::sdk::file::open(redo2, O_RDWR | O_CREAT, 0000);
        int fd_redo3 = spdb::sdk::file::open(redo3, O_RDWR | O_CREAT, 0644);
        int fd_redo4 = spdb::sdk::file::open(redo4, O_RDWR | O_CREAT, 0644);

        // Write some simulation content
        const char *msg_redo1 = "Redo log 1: test content one\n";
        const char *msg_redo2 = "Redo log 2: test content two\n";
        const char *msg_redo3 = "Redo log 3: test content three\n";
        const char *msg_redo4 = "Redo log 4: test content four\n";

        // Use pwrite to write
        spdb::sdk::file::pwrite(fd_redo1, msg_redo1, strlen(msg_redo1), 0);
        spdb::sdk::file::pwrite(fd_redo2, msg_redo2, strlen(msg_redo2), 0);
        spdb::sdk::file::pwrite(fd_redo3, msg_redo3, strlen(msg_redo3), 0);
        spdb::sdk::file::pwrite(fd_redo4, msg_redo4, strlen(msg_redo4), 0);

        // Close files
        spdb::sdk::file::close(fd_redo1);
        spdb::sdk::file::close(fd_redo2);
        spdb::sdk::file::close(fd_redo3);
        spdb::sdk::file::close(fd_redo4);

        /*
        // Delete files
        spdb::sdk::file::unlink(redo1);
        spdb::sdk::file::unlink(redo2);
        spdb::sdk::file::unlink(redo3);
        spdb::sdk::file::unlink(redo4);
        */
        // Create a MySQL supported directory
        char dir_test[] = "/mysql/data/test";
        spdb::sdk::file::mkdir(dir_test, 0755);
    }

    std::vector<FileInfo> SPDB_SDKFileSystem::list_directory(const std::string &path)
    {
        std::vector<FileInfo> files;
        std::string full_path = get_full_path(path);

        spdb::sdk::file::DIR *dir = spdb::sdk::file::opendir(full_path.c_str());
        if (!dir)
        {
            return files;
        }

        struct spdb::sdk::file::dirent *entry;
        while ((entry = spdb::sdk::file::readdir(dir)) != nullptr)
        {
            // Skip . and ..
            if (entry->d_name == "." || entry->d_name == "..")
            {
                continue;
            }

            std::string relative_file_path = (path.empty() || path == ".") ? entry->d_name : path + "/" + entry->d_name;
            try
            {
                FileInfo info = get_file_info(relative_file_path);
                info.name = entry->d_name;
                files.push_back(info);
            }
            catch (...)
            {
                // Ignore files that cannot be accessed
                continue;
            }
        }

        spdb::sdk::file::closedir(dir);

        // Sort by filename
        std::sort(files.begin(), files.end(),
                  [](const FileInfo &a, const FileInfo &b)
                  {
                      return a.name < b.name;
                  });

        return files;
    }

    bool SPDB_SDKFileSystem::is_directory(const std::string &path)
    {
        std::string full_path = get_full_path(path);
        struct stat st;
        return spdb::sdk::file::stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    bool SPDB_SDKFileSystem::exists(const std::string &path)
    {
        std::string full_path = get_full_path(path);
        struct stat st;
        return spdb::sdk::file::stat(full_path.c_str(), &st) == 0;
    }

    FileInfo SPDB_SDKFileSystem::get_file_info(const std::string &path)
    {
        std::string full_path = get_full_path(path);
        struct stat st;
        if (spdb::sdk::file::stat(full_path.c_str(), &st) != 0)
        {
            throw std::runtime_error("Cannot get file info: " + path);
        }

        FileInfo info;
        info.name = path.substr(path.find_last_of('/') + 1);
        info.type = mode_to_file_type(st.st_mode);
        info.size = st.st_size;
        info.mode = st.st_mode;
        info.mtime = st.st_mtime;
        info.atime = st.st_atime;
        info.ctime = st.st_ctime;
        info.permissions_str = FileSystemInterface::format_permissions(st.st_mode);

        return info;
    }

    FileType SPDB_SDKFileSystem::get_file_type(const std::string &path)
    {
        std::string full_path = get_full_path(path);
        struct stat st;
        if (spdb::sdk::file::stat(full_path.c_str(), &st) != 0)
        {
            return FileType::UNKNOWN;
        }
        return mode_to_file_type(st.st_mode);
    }

    off_t SPDB_SDKFileSystem::get_file_size(const std::string &path)
    {
        int fd = spdb::sdk::file::open(get_full_path(path).c_str(), O_RDONLY);
        if (fd < 0)
        {
            throw std::runtime_error("Cannot open file: " + path);
        }
        off_t size = spdb::sdk::file::file_size(fd);
        spdb::sdk::file::close(fd);
        return size;
    }

    size_t SPDB_SDKFileSystem::get_directory_size(const std::string &path, bool recursive)
    {
        if (!recursive)
        {
            return get_file_size(path);
        }
        return calculate_directory_size_recursive(path);
    }

    std::string SPDB_SDKFileSystem::read_file_content(const std::string &path, size_t max_size)
    {
        // Delegate to read_file_content_at_offset with offset 0
        return read_file_content_at_offset(path, 0, max_size);
    }

    std::string SPDB_SDKFileSystem::read_file_content_at_offset(const std::string &path, size_t offset, size_t length)
    {
        return read_file_with_fd(path, [offset, length](int fd, const std::string& path) -> std::string {
            // Get file size
            off_t file_size = spdb::sdk::file::file_size(fd);
            if (file_size < 0)
            {
                throw std::runtime_error("Failed to get file size: " + path);
            }
            
            if (offset >= static_cast<size_t>(file_size))
            {
                throw std::runtime_error("Offset exceeds file size");
            }
            
            // Determine the actual length to read
            size_t bytes_to_read = length;
            if (length == 0 || length > static_cast<size_t>(file_size) - offset)
            {
                bytes_to_read = file_size - offset;
            }
            
            std::string content;
            if (bytes_to_read > 0)
            {
                content.resize(bytes_to_read);
                size_t total_read = 0;
                while (total_read < bytes_to_read)
                {
                    ssize_t bytes_read = spdb::sdk::file::pread(
                        fd,
                        &content[total_read],
                        bytes_to_read - total_read,
                        static_cast<off_t>(offset + total_read));
                    if (bytes_read < 0)
                    {
                        throw std::runtime_error("pread failed for: " + path);
                    }
                    if (bytes_read == 0)
                        break; // EOF
                    total_read += bytes_read;
                }
                content.resize(total_read);
            }
            return content;
        });
    }

    std::string SPDB_SDKFileSystem::resolve_path(const std::string &path) const
    {
        if (path.empty())
        {
            return current_path_;
        }

        if (path[0] == '/')
        {
            return normalize_path(path); // 绝对路径
        }

        // 相对路径处理
        std::string resolved = current_path_;
        if (resolved != "/")
        {
            resolved += "/";
        }
        resolved += path;

        return normalize_path(resolved);
    }

    std::string SPDB_SDKFileSystem::get_current_directory()
    {
        return current_path_;
    }

    bool SPDB_SDKFileSystem::change_directory(const std::string &path)
    {
        std::string new_path = resolve_path(path);

        // 检查是否试图访问根目录以上
        if (!is_safe_path(new_path))
        {
            // We cannot directly output error messages here, need to let caller handle through return value
            return false;
        }

        if (is_directory(new_path))
        {
            current_path_ = new_path;
            return true;
        }
        return false;
    }

    // Private helper method implementations

    std::string SPDB_SDKFileSystem::get_full_path(const std::string &path) const
    {
        std::string normalized = normalize_path(path);

        if (normalized.empty())
        {
            return root_path_ + current_path_.substr(1); // 移除前导 /
        }

        if (normalized[0] == '/')
        {
            // Absolute path (relative to root directory)
            return root_path_ + normalized.substr(1); // Remove leading /
        }
        else
        {
            // Relative path
            std::string base_path = current_path_;
            if (base_path != "/")
            {
                base_path += "/";
            }
            base_path += normalized;
            base_path = normalize_path(base_path);
            return root_path_ + base_path.substr(1); // Remove leading /
        }
    }

    FileType SPDB_SDKFileSystem::mode_to_file_type(mode_t mode) const
    {
        if (S_ISREG(mode))
            return FileType::REGULAR_FILE;
        if (S_ISDIR(mode))
            return FileType::DIRECTORY;
        if (S_ISLNK(mode))
            return FileType::SYMBOLIC_LINK;
        if (S_ISBLK(mode))
            return FileType::BLOCK_DEVICE;
        if (S_ISCHR(mode))
            return FileType::CHARACTER_DEVICE;
        if (S_ISFIFO(mode))
            return FileType::FIFO;
        if (S_ISSOCK(mode))
            return FileType::SOCKET;
        return FileType::UNKNOWN;
    }

    size_t SPDB_SDKFileSystem::calculate_directory_size_recursive(const std::string &path) const
    {
        size_t total_size = 0;
        std::string full_path = get_full_path(path);

        spdb::sdk::file::DIR *dir = spdb::sdk::file::opendir(full_path.c_str());
        if (!dir)
        {
            return 0;
        }

        struct spdb::sdk::file::dirent *entry;
        while ((entry = spdb::sdk::file::readdir(dir)) != nullptr)
        {
            if (entry->d_name == "." || entry->d_name == "..")
            {
                continue;
            }

            std::string file_path = full_path + "/" + entry->d_name;
            struct stat st;
            if (spdb::sdk::file::stat(file_path.c_str(), &st) == 0)
            {
                if (S_ISDIR(st.st_mode))
                {
                    // Recursively calculate subdirectory size
                    total_size += calculate_directory_size_recursive(path + "/" + entry->d_name);
                }
                else
                {
                    total_size += st.st_size;
                }
            }
        }

        closedir(dir);
        return total_size;
    }

    std::string SPDB_SDKFileSystem::get_real_system_path() const
    {
        std::string current_system_path = root_path_ + current_path_.substr(1);
        if (current_system_path.back() == '/' && current_system_path.length() > 1)
        {
            current_system_path.pop_back();
        }
        return current_system_path;
    }

    std::string SPDB_SDKFileSystem::normalize_path(const std::string &path) const
    {
        if (path.empty())
            return "/";

        std::vector<std::string> components;
        std::stringstream ss(path);
        std::string component;

        // Split path
        while (std::getline(ss, component, '/'))
        {
            if (component.empty() || component == ".")
            {
                continue;
            }
            if (component == "..")
            {
                if (!components.empty())
                {
                    components.pop_back();
                }
            }
            else
            {
                components.push_back(component);
            }
        }

        // Rebuild path
        std::string result = "/";
        for (size_t i = 0; i < components.size(); ++i)
        {
            if (i > 0)
                result += "/";
            result += components[i];
        }

        return result;
    }

    bool SPDB_SDKFileSystem::is_safe_path(const std::string &path) const
    {
        std::string normalized = normalize_path(path);
        // Path must start with /, ensuring it's within root directory scope
        return !normalized.empty() && normalized[0] == '/';
    }

    bool SPDB_SDKFileSystem::is_trying_to_escape_root(const std::string &path) const
    {
        // Check if original path contains patterns trying to escape root directory
        std::string resolved = resolve_path(path);
        return !is_safe_path(resolved);
    }

} // namespace file_client