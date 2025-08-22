#pragma once

#include <string>
#include <meta/meta.pb.h>

#include "file_constant.h"



namespace spdb::sdk::file {

FileType get_file_type(const std::string& filepath);

mode_t get_os_file_mode(meta_data::OSFileType t);

std::string make_absolute_path(const std::string &path);
}

