#!/bin/bash

# File Client Tool Build Script

echo "Building File Client Tool..."

# Create build directory
mkdir -p build
cd build

# Run CMake configuration
echo "Configuring project..."
cmake ..

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Compile project
echo "Compiling project..."
make

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Compilation successful!"
echo "Executable location: build/file_client"
echo ""
echo "How to run:"
echo "  cd build"
echo "  ./file_client"
echo ""
echo "Or run directly:"
echo "  ./build/file_client"
echo ""
echo "Usage with custom root directory:"
echo "  ./build/file_client /path/to/directory"
