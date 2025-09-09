/**
 * @file main.cpp
 * @brief SPDB SDK file client tool - main program entry point
 * @author xiebaoma
 * @date 2025-08-25
 *
 * This is a SPDB SDK based file system client tool that provides Linux command-line-like
 * file operation interface. Supported features include: directory browsing, file information
 * viewing, file content display (text and hexdump formats), etc.
 */

#include <iostream>
#include <memory>
#include <exception>
#include <string>
#include <fstream>
#include <glog/logging.h>

#include "file_client.h"
#include "spdb_sdk_filesystem.h"

/**
 * @brief Program main entry point, initializes SPDB SDK file system, creates file client and starts interactive interface.
 * @param argc Number of command line arguments
 * @param argv Command line argument array
 * @return int Program exit code, 0 for success, 1 for failure
 * @note Program supports one optional command line parameter to specify root directory path
 * @example
 *   ./ops_tools /mysql/data    # Specify root directory
 *   ./ops_tools                # Use default root directory /mysql/data
 */
int main(int argc, char *argv[])
{

    FILE* devnull = fopen("/dev/null", "w");
    dup2(fileno(devnull), fileno(stdout));

    FLAGS_log_dir = "/dev/null";
    FLAGS_logtostderr = false;
    FLAGS_alsologtostderr = false;
    FLAGS_minloglevel = google::GLOG_WARNING;

    google::InitGoogleLogging(argv[0]);

    // Set default root directory
    const std::string DEFAULT_ROOT_DIRECTORY = "/mysql/data";
    try
    {
        std::string root_directory = DEFAULT_ROOT_DIRECTORY;
        // Parse command line arguments
        if (argc > 1)
        {
            root_directory = argv[1];
            // Simple parameter validation
            if (root_directory.empty())
            {
                std::cerr << "Error: Root directory cannot be empty" << std::endl;
                return 1;
            }
        }
        // Display startup information
        std::cerr << "Starting SPDB SDK File Client..." << std::endl;
        std::cerr << "Root directory: " << root_directory << std::endl;
        std::cerr << "----------------------------------------" << std::endl;
        auto filesystem = std::make_unique<file_client::SPDB_SDKFileSystem>(root_directory);
        file_client::FileClient client(std::move(filesystem));
        client.run_interactive();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "ops_tools")
                  << " [root_directory]" << std::endl;
        std::cerr << "Example: " << (argc > 0 ? argv[0] : "ops_tools")
                  << " /mysql/data" << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Fatal Error: Unknown exception occurred" << std::endl;
        return 1;
    }
    return 0;
}
