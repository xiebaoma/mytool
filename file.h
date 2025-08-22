#include <string>
#include <utility>
#include <vector>
#include <atomic>
#include <shared_mutex>
#include <mutex>
#include <array>

#include "file_constant.h"

namespace spdb::sdk::file {

class BaseFile {
public:
    
    BaseFile(const std::string& f_name, FileStatus f_status, FileType f_type);
    virtual ~BaseFile();

    std::string get_file_name() { return f_name_; }

    FileType get_file_type() const { return f_type_; }

    int64_t get_file_size() const {return file_size_.load();}

    virtual std::pair<FileErrorCode, ssize_t> pread(void* buf, size_t count, off_t offset) = 0;

protected:
    std::string f_name_;              ///< File name
    const FileType f_type_;                 ///< File type
    std::atomic<int64_t> file_size_ {0};
};

}