/**
 * xiebaoma
 * 2025-08-25
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

namespace file_client {

SPDB_SDKFileSystem::SPDB_SDKFileSystem(const std::string& root_path)
    : current_path_("/") {
    spdb::sdk::initialize("/etc/spdb/sdk_default_config.toml");

    // 转换为绝对路径
    if (char* abs_path = spdb::sdk::file::realpath(root_path.c_str(),nullptr)) {
        root_path_ = std::string(abs_path);
        free(abs_path);
    } else {
        // 如果路径不存在，尝试创建它
        if (spdb::sdk::file::mkdir(root_path.c_str(), 0755) == 0) {
            abs_path = spdb::sdk::file::realpath(root_path.c_str(), nullptr);
            if (abs_path) {
                root_path_ = std::string(abs_path);
                free(abs_path);
            } else {
                root_path_ = root_path;  // fallback
            }
        } else {
            root_path_ = root_path;  // fallback
        }
    }

    // 确保根路径以 / 结尾
    if (root_path_.back() != '/') {
        root_path_ += '/';
    }
}

void SPDB_SDKFileSystem::test_initialize() {
    char redo1[] = "/mysql/data/#ib_redo1";
    char redo2[] = "/mysql/data/#ib_redo2";
    char redo3[] = "/mysql/data/#ib_redo3";
    char redo4[] = "/mysql/data/#ib_redo4";

    // 打开文件
    int fd_redo1 = spdb::sdk::file::open(redo1, O_RDWR | O_CREAT);
    int fd_redo2 = spdb::sdk::file::open(redo2, O_RDWR | O_CREAT);
    int fd_redo3 = spdb::sdk::file::open(redo3, O_RDWR | O_CREAT);
    int fd_redo4 = spdb::sdk::file::open(redo4, O_RDWR | O_CREAT);

    // 写入一些模拟内容
    const char* msg_redo1 = "Redo log 1: test content\n";
    const char* msg_redo2 = "Redo log 2: test content\n";
    const char* msg_redo3 = "Redo log 3: test content\n";
    const char* msg_redo4 = "Redo log 4: test content\n";

    // 使用 pwrite 写入
    spdb::sdk::file::pwrite(fd_redo1, msg_redo1, strlen(msg_redo1), 0);
    spdb::sdk::file::pwrite(fd_redo2, msg_redo2, strlen(msg_redo2), 0);
    spdb::sdk::file::pwrite(fd_redo3, msg_redo3, strlen(msg_redo3), 0);
    spdb::sdk::file::pwrite(fd_redo4, msg_redo4, strlen(msg_redo4), 0);

    // 关闭文件
    spdb::sdk::file::close(fd_redo1);
    spdb::sdk::file::close(fd_redo2);
    spdb::sdk::file::close(fd_redo3);
    spdb::sdk::file::close(fd_redo4);

    /*
    //删除文件
    spdb::sdk::file::unlink(redo1);
    spdb::sdk::file::unlink(redo2);
    spdb::sdk::file::unlink(redo3);
    spdb::sdk::file::unlink(redo4);
    */
    // 创建一个 MySQL 支持的目录
    char dir_test[] = "/mysql/data/test";
    spdb::sdk::file::mkdir(dir_test, 0755);
}

std::vector<FileInfo> SPDB_SDKFileSystem::list_directory(const std::string& path) {
    std::vector<FileInfo> files;
    std::string full_path = get_full_path(path);

    spdb::sdk::file::DIR* dir = spdb::sdk::file::opendir(full_path.c_str());
    if (!dir) {
        return files;
    }

    struct spdb::sdk::file::dirent* entry;
    while ((entry = spdb::sdk::file::readdir(dir)) != nullptr) {
        // 跳过 . 和 ..
        if (entry->d_name == "." || entry->d_name == "..") {
            continue;
        }

        std::string relative_file_path = (path.empty() || path == ".") ?
            entry->d_name : path + "/" + entry->d_name;
        try {
            FileInfo info = get_file_info(relative_file_path);
            info.name = entry->d_name;
            files.push_back(info);
        } catch (...) {
            // 忽略无法访问的文件
            continue;
        }
    }

    spdb::sdk::file::closedir(dir);

    // 按文件名排序
    std::sort(files.begin(), files.end(),
              [](const FileInfo& a, const FileInfo& b) {
                  return a.name < b.name;
              });

    return files;
}

bool SPDB_SDKFileSystem::is_directory(const std::string& path) {
    std::string full_path = get_full_path(path);
    struct stat st;
    return spdb::sdk::file::stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool SPDB_SDKFileSystem::exists(const std::string& path) {
    std::string full_path = get_full_path(path);
    struct stat st;
    return spdb::sdk::file::stat(full_path.c_str(), &st) == 0;
}

FileInfo SPDB_SDKFileSystem::get_file_info(const std::string& path) {
    std::string full_path = get_full_path(path);
    struct stat st;
    if (spdb::sdk::file::stat(full_path.c_str(), &st) != 0) {
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

FileType SPDB_SDKFileSystem::get_file_type(const std::string& path) {
    std::string full_path = get_full_path(path);
    struct stat st;
    if (spdb::sdk::file::stat(full_path.c_str(), &st) != 0) {
        return FileType::UNKNOWN;
    }
    return mode_to_file_type(st.st_mode);
}

off_t SPDB_SDKFileSystem::get_file_size(const std::string& path) {
    int fd= spdb::sdk::file::open(get_full_path(path).c_str(), O_RDONLY);
    if(fd<0){
        throw std::runtime_error("Cannot open file: " + path);
    }
    off_t size= spdb::sdk::file::file_size(fd);
    spdb::sdk::file::close(fd);
    return size;
}

size_t SPDB_SDKFileSystem::get_directory_size(const std::string& path, bool recursive) {
    if (!recursive) {
        return get_file_size(path);
    }
    return calculate_directory_size_recursive(path);
}

std::string SPDB_SDKFileSystem::read_file_content(const std::string& path, size_t max_size) {
    std::string full_path = get_full_path(path);

    // 打开文件
    int fd = spdb::sdk::file::open(full_path.c_str(), O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::string content;
    try {
        if (max_size == 0) {
            // 读取整个文件（先用 lseek 获取大小）
            off_t file_size = spdb::sdk::file::lseek(fd, 0, SEEK_END);
            if (file_size < 0) {
                throw std::runtime_error("lseek failed for: " + path);
            }
            spdb::sdk::file::lseek(fd, 0, SEEK_SET); // 回到开头

            content.resize(file_size);
            size_t total_read = 0;
            while (total_read < static_cast<size_t>(file_size)) {
                ssize_t bytes_read = spdb::sdk::file::read(fd, &content[total_read], file_size - total_read);
                if (bytes_read < 0) {
                    throw std::runtime_error("read failed for: " + path);
                }
                if (bytes_read == 0) break; // EOF
                total_read += bytes_read;
            }
            content.resize(total_read);
        } else {
            // 读取指定大小
            content.resize(max_size);
            size_t total_read = 0;
            while (total_read < max_size) {
                ssize_t bytes_read = spdb::sdk::file::read(fd, &content[total_read], max_size - total_read);
                if (bytes_read < 0) {
                    throw std::runtime_error("read failed for: " + path);
                }
                if (bytes_read == 0) break; // EOF
                total_read += bytes_read;
            }
            content.resize(total_read);
        }
    } catch (...) {
        spdb::sdk::file::close(fd);
        throw;
    }

    spdb::sdk::file::close(fd);
    return content;
}

std::string SPDB_SDKFileSystem::read_file_content_at_offset(const std::string& path, size_t offset, size_t length) {
    std::string full_path = get_full_path(path);

    // 打开文件
    int fd = spdb::sdk::file::open(full_path.c_str(), O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::string content;
    try {
        // 获取文件大小以验证偏移量
        off_t file_size = spdb::sdk::file::lseek(fd, 0, SEEK_END);
        if (file_size < 0) {
            throw std::runtime_error("lseek failed for: " + path);
        }

        if (offset >= static_cast<size_t>(file_size)) {
            throw std::runtime_error("Offset exceeds file size");
        }

        // 移动到指定偏移位置
        if (spdb::sdk::file::lseek(fd, offset, SEEK_SET) < 0) {
            throw std::runtime_error("lseek to offset failed for: " + path);
        }

        // 确定要读取的实际长度
        size_t bytes_to_read = length;
        if (length == 0 || offset + length > static_cast<size_t>(file_size)) {
            bytes_to_read = file_size - offset;
        }

        if (bytes_to_read > 0) {
            content.resize(bytes_to_read);
            size_t total_read = 0;
            while (total_read < bytes_to_read) {
                ssize_t bytes_read = spdb::sdk::file::read(fd, &content[total_read], bytes_to_read - total_read);
                if (bytes_read < 0) {
                    throw std::runtime_error("read failed for: " + path);
                }
                if (bytes_read == 0) break; // EOF
                total_read += bytes_read;
            }
            content.resize(total_read);
        }
    } catch (...) {
        spdb::sdk::file::close(fd);
        throw;
    }

    spdb::sdk::file::close(fd);
    return content;
}

std::string SPDB_SDKFileSystem::resolve_path(const std::string& path) const {
    if (path.empty()) {
        return current_path_;
    }

    if (path[0] == '/') {
        return normalize_path(path); // 绝对路径
    }

    // 相对路径处理
    std::string resolved = current_path_;
    if (resolved != "/") {
        resolved += "/";
    }
    resolved += path;

    return normalize_path(resolved);
}

std::string SPDB_SDKFileSystem::get_current_directory() {
    return current_path_;
}

bool SPDB_SDKFileSystem::change_directory(const std::string& path) {
    std::string new_path = resolve_path(path);

    // 检查是否试图访问根目录以上
    if (!is_safe_path(new_path)) {
        // 这里我们无法直接输出错误信息，需要通过返回值让调用者处理
        return false;
    }

    if (is_directory(new_path)) {
        current_path_ = new_path;
        return true;
    }
    return false;
}

// 私有辅助方法实现

std::string SPDB_SDKFileSystem::get_full_path(const std::string& path) const {
    std::string normalized = normalize_path(path);

    if (normalized.empty()) {
        return root_path_ + current_path_.substr(1); // 移除前导 /
    }

    if (normalized[0] == '/') {
        // 绝对路径（相对于根目录）
        return root_path_ + normalized.substr(1); // 移除前导 /
    } else {
        // 相对路径
        std::string base_path = current_path_;
        if (base_path != "/") {
            base_path += "/";
        }
        base_path += normalized;
        base_path = normalize_path(base_path);
        return root_path_ + base_path.substr(1); // 移除前导 /
    }
}

FileType SPDB_SDKFileSystem::mode_to_file_type(mode_t mode) const {
    if (S_ISREG(mode)) return FileType::REGULAR_FILE;
    if (S_ISDIR(mode)) return FileType::DIRECTORY;
    if (S_ISLNK(mode)) return FileType::SYMBOLIC_LINK;
    if (S_ISBLK(mode)) return FileType::BLOCK_DEVICE;
    if (S_ISCHR(mode)) return FileType::CHARACTER_DEVICE;
    if (S_ISFIFO(mode)) return FileType::FIFO;
    if (S_ISSOCK(mode)) return FileType::SOCKET;
    return FileType::UNKNOWN;
}

size_t SPDB_SDKFileSystem::calculate_directory_size_recursive(const std::string& path) const {
    size_t total_size = 0;
    std::string full_path = get_full_path(path);

    spdb::sdk::file::DIR* dir = spdb::sdk::file::opendir(full_path.c_str());
    if (!dir) {
        return 0;
    }

    struct spdb::sdk::file::dirent* entry;
    while ((entry = spdb::sdk::file::readdir(dir)) != nullptr) {
        if (entry->d_name == "." || entry->d_name == "..") {
            continue;
        }

        std::string file_path = full_path + "/" + entry->d_name;
        struct stat st;
        if (spdb::sdk::file::stat(file_path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // 递归计算子目录大小
                total_size += calculate_directory_size_recursive(path + "/" + entry->d_name);
            } else {
                total_size += st.st_size;
            }
        }
    }

    closedir(dir);
    return total_size;
}


    std::string SPDB_SDKFileSystem::get_real_system_path() const {
    std::string current_system_path = root_path_ + current_path_.substr(1);
    if (current_system_path.back() == '/' && current_system_path.length() > 1) {
        current_system_path.pop_back();
    }
    return current_system_path;
}




std::string SPDB_SDKFileSystem::normalize_path(const std::string& path) const {
    if (path.empty()) return "/";

    std::vector<std::string> components;
    std::stringstream ss(path);
    std::string component;

    // 分割路径
    while (std::getline(ss, component, '/')) {
        if (component.empty() || component == ".") {
            continue;
        }
        if (component == "..") {
            if (!components.empty()) {
                components.pop_back();
            }
        } else {
            components.push_back(component);
        }
    }

    // 重建路径
    std::string result = "/";
    for (size_t i = 0; i < components.size(); ++i) {
        if (i > 0) result += "/";
        result += components[i];
    }

    return result;
}

bool SPDB_SDKFileSystem::is_safe_path(const std::string& path) const {
    std::string normalized = normalize_path(path);
    // 路径必须以 / 开头，这确保了它在根目录范围内
    return !normalized.empty() && normalized[0] == '/';
}

bool SPDB_SDKFileSystem::is_trying_to_escape_root(const std::string& path) const {
    // 检查原始路径是否包含试图逃逸根目录的模式
    std::string resolved = resolve_path(path);
    return !is_safe_path(resolved);
}


} // namespace file_client