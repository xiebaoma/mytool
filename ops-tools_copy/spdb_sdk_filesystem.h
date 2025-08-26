/**
 * @file spdb_sdk_filesystem.h
 * @brief SPDB SDK file system implementation
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 *
 * Implements file system interface using SPDB SDK.
 * Provides secure access to files and directories within a specified root directory.
 */

#pragma once

#include "filesystem_interface.h"
#include <functional>
#include <fcntl.h>
#include <spdb-sdk/file/file_io.h>

namespace file_client {

    /**
     * @class SPDB_SDKFileSystem
     * @brief SPDB SDK file system implementation
     *
     * Concrete implementation of FileSystemInterface using SPDB SDK.
     * Restricts access to a specified root directory for security.
     */
    class SPDB_SDKFileSystem : public FileSystemInterface {
    public:
        explicit SPDB_SDKFileSystem(const std::string& root_path = "test");
        ~SPDB_SDKFileSystem() override = default;
        void test_initialize();

        // Directory operations
        std::vector<FileInfo> list_directory(const std::string& path) override;
        bool is_directory(const std::string& path) override;
        bool exists(const std::string& path) override;

        // File information retrieval
        FileInfo get_file_info(const std::string& path) override;
        FileType get_file_type(const std::string& path) override;
        off_t get_file_size(const std::string& path) override;
        size_t get_directory_size(const std::string& path, bool recursive = true) override;

        // File content reading
        std::string read_file_content(const std::string& path, size_t max_size = 0) override;
        std::string read_file_content_at_offset(const std::string& path, size_t offset, size_t length = 0) override;

        // Path processing
        std::string resolve_path(const std::string& path) const override;
        std::string get_current_directory() override;
        bool change_directory(const std::string& path) override;

        // Get real system path
        std::string get_real_system_path() const;

        // Check if path tries to access above root directory
        bool is_trying_to_escape_root(const std::string& path) const;

    private:
        std::string root_path_;       ///< Absolute root path
        std::string current_path_;    ///< Current relative path (relative to root directory)

        // Helper methods
        std::string get_full_path(const std::string& path) const;
        FileType mode_to_file_type(mode_t mode) const;
        size_t calculate_directory_size_recursive(const std::string& path) const;
        std::string normalize_path(const std::string& path) const;
        bool is_safe_path(const std::string& path) const;
        
        // File I/O helper with automatic resource management
        template<typename Func>
        std::string read_file_with_fd(const std::string& path, Func&& read_func) const
        {
            std::string full_path = get_full_path(path);
            int fd = spdb::sdk::file::open(full_path.c_str(), O_RDONLY);
            if (fd < 0)
            {
                throw std::runtime_error("Cannot open file: " + path);
            }
            try
            {
                return read_func(fd, path);
            }
            catch (...)
            {
                spdb::sdk::file::close(fd);
                throw;
            }
            spdb::sdk::file::close(fd);
        }
    };
} // namespace file_client
