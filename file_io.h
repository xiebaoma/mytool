#pragma once

#include "dir.h"

namespace spdb::sdk::file {

DIR* opendir(const char* pathname);
ssize_t read(int fd, void *buf, size_t count);

} 
