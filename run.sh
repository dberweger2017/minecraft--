#!/bin/bash

# Exit on any error
set -e

# 1. Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# 2. Configure the project if not already configured
if [ ! -f "build/CMakeCache.txt" ]; then
    echo "Configuring project..."
    cmake -B build
fi

# 3. Build the project
# CMake (and the underlying build tool like 'make' or 'ninja') 
# automatically checks if the executable is the newest version 
# by comparing timestamps of source files and dependencies.
echo "Building project..."
cmake --build build

# 4. Run the executable
echo "Running minecraft_vk..."
./build/minecraft_vk
