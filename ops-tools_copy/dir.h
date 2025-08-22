#pragma once

#include <string>
#include <vector>

namespace spdb::sdk::file {
    enum DirType {
        DT_UNKNOWN = 0,
        DT_REG = 1,
        DT_DIR = 2
    };

    struct dirent {
        dirent(const std::string &d_name, DirType d_type):d_name(d_name), d_type(d_type){}
        std::string d_name;
        int d_type;
    };

    class DIR {
    public:
        void add_dir(const std::string &d_name, DirType d_type) {dirent_vec_.emplace_back(d_name, d_type);}
        dirent* scan_dir() {
            if (index_ >= dirent_vec_.size()) {
                return nullptr;
            }
            return &(dirent_vec_[index_++]);
        }

    private:
        int64_t index_ {0};
        std::vector<dirent> dirent_vec_;
    };


}