/**
 * @file filesystem_interface.h
 * @brief 文件系统抽象接口定义
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 *
 * 定义了文件系统操作的抽象接口，支持多种文件系统实现。
 * 提供了类型安全的文件操作接口和统一的错误处理机制。
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>

namespace file_client {

    /**
     * @enum FileType
     * @brief 文件类型枚举
     *
     * 定义了系统支持的各种文件类型，与Unix文件系统的文件类型对应。
     */
    enum class FileType {
        REGULAR_FILE,       ///< 普通文件
        DIRECTORY,          ///< 目录
        SYMBOLIC_LINK,      ///< 符号链接
        BLOCK_DEVICE,       ///< 块设备
        CHARACTER_DEVICE,   ///< 字符设备
        FIFO,              ///< 命名管道(FIFO)
        SOCKET,            ///< Unix域套接字
        UNKNOWN            ///< 未知类型
    };

    /**
     * @struct FileInfo
     * @brief 文件信息结构体
     *
     * 包含了文件的完整信息，包括名称、类型、大小、权限和时间戳等。
     */
    struct FileInfo {
        std::string name;           ///< 文件名
        FileType type;              ///< 文件类型
        size_t size;                ///< 文件大小（字节）
        mode_t mode;                ///< 文件权限模式
        time_t mtime;               ///< 修改时间
        time_t atime;               ///< 访问时间
        time_t ctime;               ///< 创建/状态改变时间
        std::string permissions_str; ///< 权限字符串表示（如：drwxr-xr-x）
    };

    /**
     * @class FileSystemInterface
     * @brief 文件系统抽象接口类
     *
     * 定义了文件系统操作的抽象接口，所有具体的文件系统实现都应该继承此接口。
     * 接口设计遵循RAII原则，提供异常安全的文件操作。
     *
     * @note 所有路径操作都是相对于文件系统的根目录进行的
     * @note 实现类应该提供适当的错误处理和安全检查
     */
    class FileSystemInterface {
    public:
        /**
         * @brief 虚析构函数
         */
        virtual ~FileSystemInterface() = default;

        /**
         * @name 目录操作接口
         * @{
         */

        /**
         * @brief 列出目录内容
         * @param path 目录路径
         * @return 文件信息列表
         * @throw std::runtime_error 如果目录不存在或无法访问
         */
        virtual std::vector<FileInfo> list_directory(const std::string& path) = 0;

        /**
         * @brief 检查路径是否为目录
         * @param path 文件路径
         * @return true如果是目录，false否则
         */
        virtual bool is_directory(const std::string& path) = 0;

        /**
         * @brief 检查路径是否存在
         * @param path 文件路径
         * @return true如果存在，false否则
         */
        virtual bool exists(const std::string& path) = 0;

        /** @} */

        /**
         * @name 文件信息获取接口
         * @{
         */

        /**
         * @brief 获取文件详细信息
         * @param path 文件路径
         * @return 文件信息结构体
         * @throw std::runtime_error 如果文件不存在或无法访问
         */
        virtual FileInfo get_file_info(const std::string& path) = 0;

        /**
         * @brief 获取文件类型
         * @param path 文件路径
         * @return 文件类型枚举值
         */
        virtual FileType get_file_type(const std::string& path) = 0;

        /**
         * @brief 获取文件大小
         * @param path 文件路径
         * @return 文件大小（字节）
         * @throw std::runtime_error 如果文件不存在或无法访问
         */
        virtual off_t get_file_size(const std::string& path) = 0;

        /**
         * @brief 获取目录大小
         * @param path 目录路径
         * @param recursive 是否递归计算子目录
         * @return 总大小（字节）
         * @throw std::runtime_error 如果目录不存在或无法访问
         */
        virtual size_t get_directory_size(const std::string& path, bool recursive = true) = 0;

        /** @} */

        /**
         * @name 文件内容操作接口
         * @{
         */

        /**
         * @brief 读取文件内容
         * @param path 文件路径
         * @param max_size 最大读取字节数，0表示读取整个文件
         * @return 文件内容字符串
         * @throw std::runtime_error 如果文件不存在、无法读取或文件过大
         */
        virtual std::string read_file_content(const std::string& path, size_t max_size = 0) = 0;

        /**
         * @brief 从指定偏移读取文件内容
         * @param path 文件路径
         * @param offset 读取起始偏移量
         * @param length 读取长度，0表示读取到文件末尾
         * @return 文件内容字符串
         * @throw std::runtime_error 如果文件不存在、无法读取或偏移超出文件大小
         */
        virtual std::string read_file_content_at_offset(const std::string& path, size_t offset, size_t length = 0) = 0;

        /** @} */

        /**
         * @name 路径处理接口
         * @{
         */

        /**
         * @brief 解析路径为绝对路径
         * @param path 相对或绝对路径
         * @return 解析后的绝对路径
         * @throw std::invalid_argument 如果路径格式无效
         */
        virtual std::string resolve_path(const std::string& path) const = 0;

        /**
         * @brief 获取当前工作目录
         * @return 当前目录路径
         */
        virtual std::string get_current_directory() = 0;

        /**
         * @brief 改变当前工作目录
         * @param path 目标目录路径
         * @return true如果成功，false否则
         */
        virtual bool change_directory(const std::string& path) = 0;

        /** @} */

        /**
         * @name 静态工具方法
         * @{
         */

        /**
         * @brief 格式化文件大小为可读字符串
         * @param size 文件大小（字节）
         * @param human_readable 是否使用人类可读格式（如KB、MB、GB）
         * @return 格式化的大小字符串
         */
        static std::string format_file_size(size_t size, bool human_readable = false);

        /**
         * @brief 格式化文件权限为字符串
         * @param mode 文件权限模式
         * @return 权限字符串（如：-rwxr-xr-x）
         */
        static std::string format_permissions(mode_t mode);

        /**
         * @brief 格式化时间戳为可读字符串
         * @param time 时间戳
         * @return 格式化的时间字符串
         */
        static std::string format_time(time_t time);

        /**
         * @brief 将文件类型枚举转换为字符串
         * @param type 文件类型枚举
         * @return 文件类型字符串
         */
        static std::string get_file_type_string(FileType type);

        /** @} */
    };

    /**
     * @class FileSystemFactory
     * @brief 文件系统工厂类
     *
     * 提供创建不同类型文件系统实例的工厂方法，支持扩展新的文件系统类型。
     * 使用工厂模式实现，便于维护和扩展。
     */
    class FileSystemFactory {
    public:
        /**
         * @enum Type
         * @brief 支持的文件系统类型
         */
        enum class Type {
            LINUX,      ///< Linux标准文件系统
            SPDB_SDK    ///< SPDB SDK文件系统
        };

        /**
         * @brief 创建指定类型的文件系统实例
         * @param type 文件系统类型
         * @return 文件系统接口的智能指针
         * @throw std::runtime_error 如果类型不支持
         */
        static std::unique_ptr<FileSystemInterface> create(Type type);
    };

} // namespace file_client
