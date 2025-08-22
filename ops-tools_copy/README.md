# File Client Tool Usage Guide

## Introduction

This is a file client tool designed based on spdb_sdk architecture, implementing Linux command-line-like file operations. Currently supports Linux filesystem with easy switching to spdb_sdk implementation in the future.

## compile

### bash
```bash
chmod +x build.sh
./build.sh
```

### make
```bash
mkdir -p build
cd build
cmake ..
make
```

## Running the Tool

After successful compilation:

### Default Usage (test directory as root)
```bash
./build/file_client
```

### Custom Root Directory
You can specify any directory as the root:
```bash
# Use current directory as root
./build/file_client .

# Use absolute path as root
./build/file_client /path/to/your/directory

# Use relative path as root
./build/file_client ../some_folder
```

## In the future 

### Add spdk_filesystem, implement interface
```bash
                     file_client
                          |
                          |
                          |
                filesystem_interface
                          |
                          |
               ----------   -----------    
              |                        |
              |                        |
      linux_filesystem        spdb_sdk_filesystem
              |                        |
              |                        |
          linux_api                 spdb_sdk
```
