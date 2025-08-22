#include "file_io.h"
#include "file_manager.h"
#include "file_errno_internal.h"
#include "config/sdk_config.h"

namespace spdb::sdk::file {

ssize_t pread(int fd, void *buf, size_t count, off_t offset) {
    ssize_t bytes_read = FileManager::instance().pread(fd, buf, count, offset);
    return bytes_read;
}

ssize_t read(int fd, void *buf, size_t count) {
    ssize_t bytes_read = FileManager::instance().read(fd, buf, count);
    return bytes_read;
}

off_t file_size(int fd) {
    off_t size = FileManager::instance().file_size(fd);
    return size;
}

DIR* opendir(const char* pathname) {
    return FileManager::instance().opendir(pathname);
}

} 