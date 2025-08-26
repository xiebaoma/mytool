/**
 * @file filesystem_interface.h
 * @brief File system abstract interface definition
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 *
 * Defines abstract interface for file system operations, supporting multiple file system implementations.
 * Provides type-safe file operation interfaces and unified error handling mechanism.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>

namespace file_client
{

    /**
     * @enum FileType
     * @brief File type enumeration
     *
     * Defines various file types supported by the system, corresponding to Unix file system file types.
     */
    enum class FileType
    {
        REGULAR_FILE,     ///< Regular file
        DIRECTORY,        ///< Directory
        SYMBOLIC_LINK,    ///< Symbolic link
        BLOCK_DEVICE,     ///< Block device
        CHARACTER_DEVICE, ///< Character device
        FIFO,             ///< Named pipe (FIFO)
        SOCKET,           ///< Unix domain socket
        UNKNOWN           ///< Unknown type
    };

    /**
     * @struct FileInfo
     * @brief File information structure
     *
     * Contains complete file information including name, type, size, permissions and timestamps.
     */
    struct FileInfo
    {
        std::string name;            ///< File name
        FileType type;               ///< File type
        size_t size;                 ///< File size (bytes)
        mode_t mode;                 ///< File permission mode
        time_t mtime;                ///< Modification time
        time_t atime;                ///< Access time
        time_t ctime;                ///< Creation/status change time
        std::string permissions_str; ///< Permission string representation (e.g.: drwxr-xr-x)
    };

    /**
     * @class FileSystemInterface
     * @brief File system abstract interface class
     *
     * Defines abstract interface for file system operations, all concrete file system
     * implementations should inherit from this interface. Interface design follows RAII
     * principles and provides exception-safe file operations.
     *
     * @note All path operations are relative to the file system root directory
     * @note Implementation classes should provide appropriate error handling and security checks
     */
    class FileSystemInterface
    {
    public:
        /**
         * @brief Virtual destructor
         */
        virtual ~FileSystemInterface() = default;

        /**
         * @name Directory operation interface
         * @{
         */

        /**
         * @brief List directory contents
         * @param path Directory path
         * @return File information list
         * @throw std::runtime_error if directory does not exist or cannot be accessed
         */
        virtual std::vector<FileInfo> list_directory(const std::string &path) = 0;

        /**
         * @brief Check if path is a directory
         * @param path File path
         * @return true if it's a directory, false otherwise
         */
        virtual bool is_directory(const std::string &path) = 0;

        /**
         * @brief Check if path exists
         * @param path File path
         * @return true if exists, false otherwise
         */
        virtual bool exists(const std::string &path) = 0;

        /** @} */

        /**
         * @name File information retrieval interface
         * @{
         */

        /**
         * @brief Get detailed file information
         * @param path File path
         * @return File information structure
         * @throw std::runtime_error if file does not exist or cannot be accessed
         */
        virtual FileInfo get_file_info(const std::string &path) = 0;

        /**
         * @brief Get file type
         * @param path File path
         * @return File type enumeration value
         */
        virtual FileType get_file_type(const std::string &path) = 0;

        /**
         * @brief Get file size
         * @param path File path
         * @return File size (bytes)
         * @throw std::runtime_error if file does not exist or cannot be accessed
         */
        virtual off_t get_file_size(const std::string &path) = 0;

        /**
         * @brief Get directory size
         * @param path Directory path
         * @param recursive Whether to recursively calculate subdirectories
         * @return Total size (bytes)
         * @throw std::runtime_error if directory does not exist or cannot be accessed
         */
        virtual size_t get_directory_size(const std::string &path, bool recursive = true) = 0;

        /** @} */

        /**
         * @name File content operation interface
         * @{
         */

        /**
         * @brief Read file content
         * @param path File path
         * @param max_size Maximum bytes to read, 0 means read entire file
         * @return File content string
         * @throw std::runtime_error if file does not exist, cannot be read, or file is too large
         */
        virtual std::string read_file_content(const std::string &path, size_t max_size = 0) = 0;

        /**
         * @brief Read file content from specified offset
         * @param path File path
         * @param offset Starting read offset
         * @param length Read length, 0 means read to end of file
         * @return File content string
         * @throw std::runtime_error if file does not exist, cannot be read, or offset exceeds file size
         */
        virtual std::string read_file_content_at_offset(const std::string &path, size_t offset, size_t length = 0) = 0;

        /** @} */

        /**
         * @name Path processing interface
         * @{
         */

        /**
         * @brief Resolve path to absolute path
         * @param path Relative or absolute path
         * @return Resolved absolute path
         * @throw std::invalid_argument if path format is invalid
         */
        virtual std::string resolve_path(const std::string &path) const = 0;

        /**
         * @brief Get current working directory
         * @return Current directory path
         */
        virtual std::string get_current_directory() = 0;

        /**
         * @brief Change current working directory
         * @param path Target directory path
         * @return true if successful, false otherwise
         */
        virtual bool change_directory(const std::string &path) = 0;

        /** @} */

        /**
         * @name Static utility methods
         * @{
         */

        /**
         * @brief Format file size to readable string
         * @param size File size (bytes)
         * @param human_readable Whether to use human-readable format (like KB, MB, GB)
         * @return Formatted size string
         */
        static std::string format_file_size(size_t size, bool human_readable = false);

        /**
         * @brief Format file permissions to string
         * @param mode File permission mode
         * @return Permission string (e.g.: -rwxr-xr-x)
         */
        static std::string format_permissions(mode_t mode);

        /**
         * @brief Format timestamp to readable string
         * @param time Timestamp
         * @return Formatted time string
         */
        static std::string format_time(time_t time);

        /**
         * @brief Convert file type enumeration to string
         * @param type File type enumeration
         * @return File type string
         */
        static std::string get_file_type_string(FileType type);

        /** @} */
    };

    /**
     * @class FileSystemFactory
     * @brief File system factory class
     *
     * Provides factory methods for creating different types of file system instances,
     * supporting extension of new file system types. Uses factory pattern for
     * easy maintenance and extension.
     */
    class FileSystemFactory
    {
    public:
        /**
         * @enum Type
         * @brief Supported file system types
         */
        enum class Type
        {
            LINUX,   ///< Linux standard file system
            SPDB_SDK ///< SPDB SDK file system
        };

        /**
         * @brief Create file system instance of specified type
         * @param type File system type
         * @return Smart pointer to file system interface
         * @throw std::runtime_error if type is not supported
         */
        static std::unique_ptr<FileSystemInterface> create(Type type);
    };

} // namespace file_client
