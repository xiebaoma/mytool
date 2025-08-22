#pragma once

#include <atomic>
#include <string>


// 前向声明或包含必要的头文件
namespace spdb::sdk::ps {
    class PageServerIOManager;
}

namespace spdb::sdk {
//     

// 初始化和清理接口
extern std::atomic<bool> g_initialized;

bool initialize(const std::string& config_path);

bool initialize_without_meta_check(const std::string& config_path);

/**
 * @brief 初始化页面服务器SDK
 * 
 * 该函数初始化 PageServer SDK，包括：
 * 1. 物理I/O管理器 (PhysicalFileIO)  
 * 2. 元数据管理器 (MetaManager)
 * 3. 页面服务器I/O管理器 (PageServerIOManager)
 * 
 * @param sdk_config_path SDK配置文件路径
 * @param storage_config_path 存储配置文件路径
 * @return true 初始化成功，false 初始化失败
 */
bool initialize_page_server_sdk(const std::string& sdk_config_path);
}