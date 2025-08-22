#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sys/stat.h>

namespace file_client {

// 文件类型枚举
enum class FileType {
    REGULAR_FILE,
    DIRECTORY,
    SYMBOLIC_LINK,
    BLOCK_DEVICE,
    CHARACTER_DEVICE,
    FIFO,
    SOCKET,
    UNKNOWN
};

// 文件信息结构体
struct FileInfo {
    std::string name;
    FileType type;
    size_t size;
    mode_t mode;
    time_t mtime;
    time_t atime;
    time_t ctime;
    std::string permissions_str;
};

// 抽象文件系统接口
class FileSystemInterface {
public:
    virtual ~FileSystemInterface() = default;

    // 目录操作
    virtual std::vector<FileInfo> list_directory(const std::string& path) = 0;
    virtual bool is_directory(const std::string& path) = 0;
    virtual bool exists(const std::string& path) = 0;
    
    // 文件信息获取
    virtual FileInfo get_file_info(const std::string& path) = 0;
    virtual FileType get_file_type(const std::string& path) = 0;
    virtual size_t get_file_size(const std::string& path) = 0;
    virtual size_t get_directory_size(const std::string& path, bool recursive = true) = 0;
    
    // 文件内容读取
    virtual std::string read_file_content(const std::string& path, size_t max_size = 0) = 0;
    
    // 路径处理
    virtual std::string resolve_path(const std::string& path) const = 0;
    virtual std::string get_current_directory() = 0;
    virtual bool change_directory(const std::string& path) = 0;
    
    // 工具方法
    static std::string format_file_size(size_t size, bool human_readable = false);
    static std::string format_permissions(mode_t mode);
    static std::string format_time(time_t time);
    static std::string get_file_type_string(FileType type);
};

// 文件系统工厂
class FileSystemFactory {
public:
    enum class Type {
        LINUX,
        SPDB_SDK
    };
    
    static std::unique_ptr<FileSystemInterface> create(Type type);
};

} // namespace file_client
