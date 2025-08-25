/**
 * @file file_client.h
 * @brief 文件客户端类定义
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 *
 * 提供类似Unix命令行的文件系统操作界面，支持交互式和批处理模式。
 * 基于文件系统接口抽象，可以适配不同的文件系统实现。
 */

#pragma once

#include "filesystem_interface.h"
#include <memory>
#include <string>
#include <vector>

namespace file_client {

    /**
     * @struct CommandResult
     * @brief 命令执行结果结构
     *
     * 封装了命令执行的结果，包括成功状态和消息信息。
     */
    struct CommandResult {
        bool success;           ///< 命令是否成功执行
        std::string message;    ///< 执行结果消息或错误信息

        /**
         * @brief 构造函数
         * @param s 成功标志，默认为true
         * @param msg 消息内容，默认为空
         */
        CommandResult(bool s = true, const std::string& msg = "")
            : success(s), message(msg) {}
    };

    /**
     * @class FileClient
     * @brief 文件系统客户端主类
     *
     * 提供类似Unix命令行工具的文件系统操作接口，支持ls、cat、stat、du等常用命令。
     * 采用命令模式设计，便于扩展新的命令。支持交互式和非交互式两种运行模式。
     *
     * @note 所有命令操作都通过FileSystemInterface接口进行，保证了与具体文件系统实现的解耦
     * @note 提供了安全的路径处理和错误处理机制
     */
    class FileClient {
    public:
        /**
         * @brief 构造函数
         * @param fs 文件系统接口的智能指针
         * @throw std::invalid_argument 如果文件系统接口为空
         */
        explicit FileClient(std::unique_ptr<FileSystemInterface> fs);

        /**
         * @brief 析构函数
         */
        ~FileClient() = default;

        /**
         * @name 命令处理接口
         * @{
         */

        /**
         * @brief 执行命令行字符串
         * @param command_line 完整的命令行字符串
         * @return 命令执行结果
         *
         * 解析命令行字符串并调用相应的命令处理方法。
         * 支持的命令包括：ls, file, stat, du, cat, cd, pwd, help, exit/quit
         */
        CommandResult execute_command(const std::string& command_line);

        /** @} */

        /**
         * @name 具体命令实现
         * @{
         */

        /**
         * @brief ls命令 - 列出目录内容
         * @param args 命令参数，支持-l选项和路径参数
         * @return 命令执行结果
         *
         * 支持的用法：
         * - ls [path]：列出指定路径的内容
         * - ls -l [path]：以详细格式列出内容
         */
        CommandResult cmd_ls(const std::vector<std::string>& args);

        /**
         * @brief file命令 - 显示文件类型
         * @param args 命令参数，第一个参数为文件路径
         * @return 命令执行结果
         */
        CommandResult cmd_file(const std::vector<std::string>& args);

        /**
         * @brief stat命令 - 显示文件详细信息
         * @param args 命令参数，第一个参数为文件路径
         * @return 命令执行结果
         */
        CommandResult cmd_stat(const std::vector<std::string>& args);

        /**
         * @brief du命令 - 显示目录大小
         * @param args 命令参数，支持-h选项和路径参数
         * @return 命令执行结果
         *
         * 支持的用法：
         * - du [path]：显示路径大小（字节）
         * - du -h [path]：以人类可读格式显示大小
         */
        CommandResult cmd_du(const std::vector<std::string>& args);

        /**
         * @brief cat命令 - 显示文件内容
         * @param args 命令参数，第一个参数为文件路径
         * @return 命令执行结果
         *
         * @note 自动检测二进制文件并拒绝显示
         * @note 限制最大文件大小以防止内存问题
         */
        CommandResult cmd_cat(const std::vector<std::string>& args);

        /**
         * @brief cd命令 - 改变当前目录
         * @param args 命令参数，第一个参数为目标目录路径
         * @return 命令执行结果
         */
        CommandResult cmd_cd(const std::vector<std::string>& args);

        /**
         * @brief pwd命令 - 显示当前工作目录
         * @param args 命令参数（忽略）
         * @return 命令执行结果
         */
        CommandResult cmd_pwd(const std::vector<std::string>& args);

        /**
         * @brief hexdump命令 - 获取偏移地址和十六进制字节
         * @param args 命令参数（文件路径）
         * @return 命令执行结果
         */
        CommandResult cmd_hexdump(const std::vector<std::string>& args);

        /**
         * @brief help命令 - 显示帮助信息
         * @param args 命令参数（忽略）
         * @return 命令执行结果
         */
        CommandResult cmd_help(const std::vector<std::string>& args);

        /** @} */

        /**
         * @name 运行模式接口
         * @{
         */

        /**
         * @brief 启动交互式模式
         *
         * 进入命令行循环，接受用户输入并执行相应命令，
         * 直到用户输入exit或quit命令退出。
         */
        void run_interactive();

        /** @} */

        /**
         * @name 辅助方法
         * @{
         */

        /**
         * @brief 获取命令行提示符
         * @return 当前的命令行提示符字符串
         */
        std::string get_prompt();

        /** @} */

    private:
        std::unique_ptr<FileSystemInterface> filesystem_;   ///< 文件系统接口指针

        /**
         * @name 私有工具方法
         * @{
         */

        /**
         * @brief 解析命令行字符串
         * @param command_line 原始命令行字符串
         * @return 分割后的参数列表
         */
        std::vector<std::string> parse_command(const std::string& command_line);

        /**
         * @brief 打印帮助信息（已废弃，使用cmd_help替代）
         * @deprecated 使用cmd_help方法替代
         */
        void print_help();

        /**
         * @brief 根据文件扩展名获取MIME类型
         * @param filename 文件名
         * @return MIME类型字符串，如果无法确定则返回空字符串
         */
        std::string get_file_mime_type(const std::string& filename);

        /**
         * @brief 检测文件内容是否为文本
         * @param content 文件内容
         * @return true如果是文本文件，false如果是二进制文件
         *
         * 使用启发式方法检测文件是否包含文本内容：
         * - 检查NULL字符的存在
         * - 计算非可打印字符的比例
         */
        bool is_text_file(const std::string& content);

        /** @} */
    };

} // namespace file_client
