/**
* xiebaoma
 * 2025-08-25
 */

#pragma once

#include "filesystem_interface.h"

namespace file_client {

    class SPDB_SDKFileSystem : public FileSystemInterface {
    public:
        explicit SPDB_SDKFileSystem(const std::string& root_path = "test");
        ~SPDB_SDKFileSystem() override = default;
        void test_initialize();

        // 目录操作
        std::vector<FileInfo> list_directory(const std::string& path) override;
        bool is_directory(const std::string& path) override;
        bool exists(const std::string& path) override;

        //文件信息获取
        FileInfo get_file_info(const std::string& path) override;
        FileType get_file_type(const std::string& path) override;
        off_t get_file_size(const std::string& path) override;
        size_t get_directory_size(const std::string& path, bool recursive = true) override;

        // 文件内容读取
        std::string read_file_content(const std::string& path, size_t max_size = 0) override;
        std::string read_file_content_at_offset(const std::string& path, size_t offset, size_t length = 0) override;

        // 路径处理
        std::string resolve_path(const std::string& path) const override;
        std::string get_current_directory() override;
        bool change_directory(const std::string& path) override;

        // 获取真实系统路径
        std::string get_real_system_path() const;

        // 检查路径是否试图访问根目录以上
        bool is_trying_to_escape_root(const std::string& path) const;

    private:
        std::string root_path_;       // 绝对根路径
        std::string current_path_;    // 当前相对路径（相对于根目录）

        // 辅助方法
        std::string get_full_path(const std::string& path) const;
        FileType mode_to_file_type(mode_t mode) const;
        size_t calculate_directory_size_recursive(const std::string& path) const;
        std::string normalize_path(const std::string& path) const;
        bool is_safe_path(const std::string& path) const;

    };

} // namespace file_client
