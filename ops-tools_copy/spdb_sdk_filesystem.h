/**
 * xiebaoma
 * 2025-08-22
 */

#pragma once

#include "filesystem_interface.h"

namespace file_client {

class SPDB_SDKFileSystem : public FileSystemInterface {
public:
    explicit SPDB_SDKFileSystem(const std::string& root_path = "test");
    
    
private:
    
};

} // namespace file_client
