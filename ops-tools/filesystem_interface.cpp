/**
 * @file filesystem_interface.cpp
 * @brief File system interface implementation
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 *
 * Implementation of static utility methods and factory methods
 * for the file system interface.
 */

#include "filesystem_interface.h"
#include "spdb_sdk_filesystem.h"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace file_client
{

    // Static utility method implementations
    std::string FileSystemInterface::format_file_size(size_t size, bool human_readable)
    {
        if (!human_readable)
        {
            return std::to_string(size);
        }

        const char *units[] = {"B", "KB", "MB", "GB", "TB"};
        double size_d = static_cast<double>(size);
        int unit_idx = 0;

        while (size_d >= 1024.0 && unit_idx < 4)
        {
            size_d /= 1024.0;
            unit_idx++;
        }

        std::ostringstream oss;
        if (unit_idx == 0)
        {
            oss << static_cast<size_t>(size_d) << units[unit_idx];
        }
        else
        {
            oss << std::fixed << std::setprecision(1) << size_d << units[unit_idx];
        }
        return oss.str();
    }

    std::string FileSystemInterface::format_permissions(mode_t mode)
    {
        std::string perm(10, '-');

        // File type
        if (S_ISDIR(mode))
            perm[0] = 'd';
        else if (S_ISLNK(mode))
            perm[0] = 'l';
        else if (S_ISBLK(mode))
            perm[0] = 'b';
        else if (S_ISCHR(mode))
            perm[0] = 'c';
        else if (S_ISFIFO(mode))
            perm[0] = 'p';
        else if (S_ISSOCK(mode))
            perm[0] = 's';

        // User permissions
        if (mode & S_IRUSR)
            perm[1] = 'r';
        if (mode & S_IWUSR)
            perm[2] = 'w';
        if (mode & S_IXUSR)
            perm[3] = 'x';

        // Group permissions
        if (mode & S_IRGRP)
            perm[4] = 'r';
        if (mode & S_IWGRP)
            perm[5] = 'w';
        if (mode & S_IXGRP)
            perm[6] = 'x';

        // Other permissions
        if (mode & S_IROTH)
            perm[7] = 'r';
        if (mode & S_IWOTH)
            perm[8] = 'w';
        if (mode & S_IXOTH)
            perm[9] = 'x';

        return perm;
    }

    std::string FileSystemInterface::format_time(time_t time)
    {
        char buffer[80];
        struct tm *tm_info = localtime(&time);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
        return std::string(buffer);
    }

    std::string FileSystemInterface::get_file_type_string(FileType type)
    {
        switch (type)
        {
        case FileType::REGULAR_FILE:
            return "regular file";
        case FileType::DIRECTORY:
            return "directory";
        case FileType::SYMBOLIC_LINK:
            return "symbolic link";
        case FileType::BLOCK_DEVICE:
            return "block device";
        case FileType::CHARACTER_DEVICE:
            return "character device";
        case FileType::FIFO:
            return "FIFO";
        case FileType::SOCKET:
            return "socket";
        default:
            return "unknown";
        }
    }

    // Factory method implementations
    std::unique_ptr<FileSystemInterface> FileSystemFactory::create(Type type)
    {
        switch (type)
        {
        // case Type::LINUX:
        // return std::make_unique<LinuxFileSystem>();
        case Type::SPDB_SDK:
            return std::make_unique<SPDB_SDKFileSystem>();
        default:
            throw std::runtime_error("Unknown filesystem type");
        }
    }

} // namespace file_client
