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

## Usage Examples

After starting the program, you'll see an interactive command line interface:

### Example Session

```
File Client Tool started (Root directory: /Users/user/project/test)
Type 'help' for available commands, 'exit' to quit

[/Users/user/project/test] / $ ls
config.json  data  documents  hello.txt  script.py

[/Users/user/project/test] / $ ls -l
-rw-r--r--        484 2024-01-20 10:30:45 config.json
drwxr-xr-x          0 2024-01-20 10:30:45 data
drwxr-xr-x          0 2024-01-20 10:30:45 documents  
-rw-r--r--        123 2024-01-20 10:30:45 hello.txt
-rw-r--r--        245 2024-01-20 10:30:45 script.py

[/Users/user/project/test] / $ file hello.txt
hello.txt: regular file, text file (text/plain)

[/Users/user/project/test] / $ stat hello.txt
File: hello.txt
Type: regular file
Size: 123 bytes
Permissions: -rw-r--r-- (0644)
Modified: 2024-01-20 10:30:45
Accessed: 2024-01-20 10:30:45
Created: 2024-01-20 10:30:45

[/Users/user/project/test] / $ du -h documents
1.2KB   documents

[/Users/user/project/test] / $ cat hello.txt
Hello, World!
This is a test text file.
It contains multiple lines of text.
Content for testing the file client tool.

This is the second paragraph.

[/Users/user/project/test] / $ cd documents
[/Users/user/project/test/documents] /documents $ ls
readme.md

[/Users/user/project/test/documents] /documents $ cd ..
[/Users/user/project/test] / $ pwd
/

[/Users/user/project/test] / $ help
File Client Tool - Available Commands:

Directory Operations:
  ls [path]          List directory contents
  ls -l [path]       List detailed directory contents (permissions, size, time)
  cd [path]          Change directory
  pwd                Show current directory

File Information:
  file <filename>    Show file type
  stat <filename>    Show detailed file information
  du [path]          Show file/directory size (bytes)
  du -h [path]       Show human-readable size (KB/MB/GB)

File Content:
  cat <filename>     Display file content

Other:
  help               Show this help message
  exit/quit          Exit the program

Note: Access is restricted to the specified root directory

[/Users/user/project/test] / $ exit
Goodbye!
```

## Security Features

### Root Directory Protection

The tool prevents navigation above the specified root directory and provides clear warning messages:

```
[/Users/user/project/test] / $ cd ..
Access denied: Cannot navigate above the root directory (..).
Current root directory restricts access to its subdirectories only.

[/Users/user/project/test] / $ cd ../../etc  
Access denied: Cannot navigate above the root directory (../../etc).
Current root directory restricts access to its subdirectories only.

[/Users/user/project/test] / $ cd /etc
Access denied: Cannot navigate above the root directory (/etc).
Current root directory restricts access to its subdirectories only.
```
