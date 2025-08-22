#pragma once

#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>


#include "dir.h"

namespace spdb::sdk::file {

// 文件操作接口
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);

int create_with_space_id(const char *pathname, int64_t space_id = -1);

int close(int fd);

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
off_t lseek(int fd, off_t offset, int whence);
ssize_t read(int fd, void *buf, size_t count);

int posix_fallocate(int fd, off_t offset, off_t len);
int fallocate(int fd, int mode, off_t offset, off_t len);

int unlink(const char *pathname);
int rename(const char *oldpath, const char *newpath);

int stat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);

off_t file_size(int fd);

int fsync(int fd);
int fdatasync(int fd);

std::string get_path_use_fd(int fd);

uint64_t get_free_space();
uint64_t get_volume_total_space();
uint64_t get_volume_use_space();

int mkdir(const char* pathname, mode_t mode);

int rmdir(const char* pathname);
DIR* opendir(const char* pathname);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

int copy_to(int fd, const char* pathname, int flags, mode_t mode);

char *realpath(const char *path, char *resolved_path);

void manual_delete_loop();

int file_exist_rw_version(const char *path);

} // namespace spdb::sdk::file 
