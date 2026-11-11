#!/bin/bash

echo "========================="
echo "Choose build method:"
echo "[1] Ninja (with compile_commands.json)"
echo "========================="
read -p "Enter 1: " BUILD_METHOD

if [ "$BUILD_METHOD" == "1" ]; then
    echo "Using Ninja..."
    
    # Create and enter the build directory
    mkdir -p out
    cd out
    # Generate the build files (note that .. means the parent directory, look for CMakeLists.txt)
    cmake .. -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    # Start compiling
    cmake --build .
fi