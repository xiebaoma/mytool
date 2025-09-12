/**
 * @file file_client.h
 * @brief File client class definition
 * @author xiebaoma
 * @date 2025-08-25
 * @version 1.0.0
 *
 * Provides Unix command-line-like file system operation interface, supporting
 * interactive modes. Based on file system interface abstraction,
 * it can adapt to different file system implementations.
 */

#pragma once

#include "filesystem_interface.h"
#include <memory>
#include <string>
#include <vector>

namespace file_client
{

    /**
     * @struct CommandResult
     * @brief Command execution result structure
     *
     * Encapsulates the result of command execution, including success status and message information.
     */
    struct CommandResult
    {
        bool success;        ///< Whether the command executed successfully
        std::string message; ///< Execution result message or error information

        /**
         * @brief Constructor
         * @param s Success flag, defaults to true
         * @param msg Message content, defaults to empty
         */
        CommandResult(bool s = true, const std::string &msg = "")
            : success(s), message(msg) {}
    };

    /**
     * @class FileClient
     * @brief File system client main class
     *
     * Provides Unix command-line-like file system operation interface, supporting
     * common commands like ls, cat, stat, etc. Uses command pattern design
     * for easy extension of new commands. Supports both interactive and non-interactive modes.
     *
     * @note All command operations are performed through the FileSystemInterface,
     *       ensuring decoupling from specific file system implementations
     * @note Provides safe path processing and error handling mechanisms
     */
    class FileClient
    {
    public:
        /**
         * @brief Constructor
         * @param fs Smart pointer to file system interface
         * @throw std::invalid_argument if file system interface is null
         */
        explicit FileClient(std::unique_ptr<FileSystemInterface> fs);

        /**
         * @brief Destructor
         */
        ~FileClient() = default;

        /**
         * @name Command processing interface
         * @{
         */

        /**
         * @brief Execute command line string
         * @param command_line Complete command line string
         * @return Command execution result
         *
         * Parses command line string and calls corresponding command handler.
         * Supported commands include: ls, file, stat, cat, cd, pwd, help, exit/quit
         */
        CommandResult execute_command(const std::string &command_line);

        /** @} */

        /**
         * @name Specific command implementations
         * @{
         */


        CommandResult cmd_ll(const std::vector<std::string> &args);


        /**
         * @brief ls command - List directory contents
         * @param args Command arguments, supports -l option and path parameter
         * @return Command execution result
         *
         * Supported usage:
         * - ls [path]: List contents of specified path
         * - ls -l [path]: List contents in detailed format
         */
        CommandResult cmd_ls(const std::vector<std::string> &args);

        /**
         * @brief file command - Display file type
         * @param args Command arguments, first argument is file path
         * @return Command execution result
         */
        CommandResult cmd_file(const std::vector<std::string> &args);

        /**
         * @brief stat command - Display detailed file information
         * @param args Command arguments, first argument is file path
         * @return Command execution result
         */
        CommandResult cmd_stat(const std::vector<std::string> &args);

        /**
         * This feature is temporarily disabled
         *
         * @brief du command - Display directory size
         * @param args Command arguments, supports -h option and path parameter
         * @return Command execution result
         *
         * Supported usage:
         * - du [path]: Display path size in bytes
         * - du -h [path]: Display size in human-readable format
         */
        CommandResult cmd_du(const std::vector<std::string> &args);

        /**
         * @brief cat command - Display file contents
         * @param args Command arguments, first argument is file path
         * @return Command execution result
         *
         * @note Automatically detects binary files and refuses to display them
         * @note Limits maximum file size to prevent memory issues
         */
        CommandResult cmd_cat(const std::vector<std::string> &args);

        /**
         * @brief cd command - Change current directory
         * @param args Command arguments, first argument is target directory path
         * @return Command execution result
         */
        CommandResult cmd_cd(const std::vector<std::string> &args);

        /**
         * @brief pwd command - Display current working directory
         * @param args Command arguments (ignored)
         * @return Command execution result
         */
        CommandResult cmd_pwd(const std::vector<std::string> &args);

        /**
         * @brief hexdump command - Get offset addresses and hexadecimal bytes
         * @param args Command arguments (file path)
         * @return Command execution result
         */
        CommandResult cmd_hexdump(const std::vector<std::string> &args);

        /**
         * @brief Get file metadata command
         * @param args Command arguments (filename)
         * @return Command execution result
         * @note Only supports redolog and IBD files with metadata
         */
        CommandResult cmd_meta(const std::vector<std::string> &args);

        /**
         * @brief help command - Display help information
         * @param args Command arguments (ignored)
         * @return Command execution result
         */
        CommandResult cmd_help(const std::vector<std::string> &args);

        /** @} */

        /**
         * @name Runtime mode interface
         * @{
         */

        /**
         * @brief Start interactive mode
         *
         * Enters command line loop, accepting user input and executing corresponding commands,
         * until user inputs exit or quit command to exit.
         */
        void run_interactive();

        /** @} */

        /**
         * @name Helper methods
         * @{
         */

        /**
         * @brief Get command line prompt
         * @return Current command line prompt string
         */
        std::string get_prompt();

        /** @} */

    private:
        std::unique_ptr<FileSystemInterface> filesystem_; ///< File system interface pointer

        /**
         * @name Private utility methods
         * @{
         */

        /**
         * @brief Parse command line string
         * @param command_line Raw command line string
         * @return Split argument list
         */
        std::vector<std::string> parse_command(const std::string &command_line);

        /**
         * @brief Print help information (deprecated, use cmd_help instead)
         * @deprecated Use cmd_help method instead
         */
        void print_help();

        /**
         * @brief Get MIME type based on file extension
         * @param filename File name
         * @return MIME type string, returns empty string if cannot be determined
         */
        std::string get_file_mime_type(const std::string &filename);

        /**
         * @brief Detect if file content is text
         * @param content File content
         * @return true if it's a text file, false if it's a binary file
         *
         * Uses heuristic methods to detect if file contains text content:
         * - Checks for presence of NULL characters
         * - Calculates ratio of non-printable characters
         */
        bool is_text_file(const std::string &content);

        /** @} */
    };

} // namespace file_client
