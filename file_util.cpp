#include "file_util.h"

#include <meta/meta.pb.h>
#include <sys/stat.h>
#include <glog/logging.h>

namespace spdb::sdk::file {

    FileType get_file_type(const std::string &filepath) {
        // 获取文件名(去掉路径)
        size_t pos = filepath.find_last_of("/\\");
        std::string filename = (pos == std::string::npos) ?
                               filepath : filepath.substr(pos + 1);

        // 检查是否是redo log文件 (#ib_redo 前缀)
        if (filename.compare(0, 8, "#ib_redo") == 0) {
            return FileType::REDO;
        }

        // 获取文件扩展名
        pos = filename.find_last_of('.');
        if (pos != std::string::npos) {
            std::string ext = filename.substr(pos);

            // 检查是否是data文件 (.ibd后缀)
            if (ext == ".ibd" || ext == ".ibt") {
                return FileType::DATA;
            }

            // 检查是否是double write文件 (.dblwr后缀)
            if (ext == ".dblwr") {
                return FileType::DOUBLE_WRITE;
            }

            if (ext == ".ibu") {
                return FileType::UNDO;
            }
        }

        // 最后检查是不是系统生成的undo，格式为 undo_
        if (filename.compare(0, 5, "undo_") == 0) {
            return FileType::UNDO;
        }

        // 默认返回未知类型
        return FileType::UNKNOWN;
    }

    mode_t get_os_file_mode(meta_data::OSFileType t) {
        constexpr mode_t m = 0;
        if (t == meta_data::NORMAL) {
            return m | S_IFREG;
        } else if (t == meta_data::DIR) {
            return m | S_IFDIR;
        } else {
            // 默认统一返回普通文件
            return m | S_IFREG;
        }
    }

    std::string make_absolute_path(const std::string &rpath) {
        if (rpath == "/") {
            return rpath;
        }

        std::string path;
        if (!rpath.empty() && rpath.back() == '/') {
            path = rpath.substr(0, rpath.size() - 1);
        } else {
            path = rpath;
        }

        // 绝对路径直接返回
        if (!path.empty() && path[0] == '/') {
            return path;
        }

        // 如果路径包含 "../"，打印错误并返回 ""
        if (path.rfind("../", 0) == 0) {
            //std::cerr << "Error: Relative path contains '../', operation not allowed: " << path << std::endl;
            return "";
        }

        // 获取当前进程的工作目录
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) {
            perror("getcwd failed");
            return "";
        }

        std::string abs_path;
        if (path.rfind("./", 0) == 0) {  // 以 "./" 开头
            abs_path = std::string(cwd) + path.substr(1);  // 去掉 "./"
        } else if (path == ".") {
            abs_path = std::string(cwd);
        } else {  // 直接是 "file.txt" 这种相对路径
            abs_path = std::string(cwd) + "/" + path;
        }

        return abs_path;
    }

}
