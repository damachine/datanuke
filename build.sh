#!/bin/bash

# ==============================================================================
# DataNuke - Secure Data Deletion Tool
# Makes data powerless
#
# Quick Build Script for Unix-like systems (Linux, macOS)
# ==============================================================================
#
# This script provides a convenient way to build DataNuke using CMake.
# It performs the following steps:
#   1. Creates a build directory (if it doesn't exist)
#   2. Runs CMake to generate Makefiles
#   3. Compiles the project using all available CPU cores
#
# Requirements:
#   - CMake 3.15 or higher
#   - GCC 7+ or Clang 10+ (C11 support required)
#   - OpenSSL development libraries (libssl-dev on Debian/Ubuntu)
#
# Usage:
#   ./build.sh           # Build in Debug mode (default)
#   BUILD_TYPE=Release ./build.sh  # Build in Release mode
#
# ==============================================================================

set -e  # Exit on any error

echo "========================================"
echo "  DataNuke Build System"
echo "  Makes data powerless"
echo "========================================"
echo ""

# Determine build type (Debug or Release)
BUILD_TYPE=${BUILD_TYPE:-Debug}
echo "Build type: $BUILD_TYPE"
echo ""

# Create build directory if it doesn't exist
# Using out-of-source build to keep source directory clean
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Run CMake to generate build files
# CMAKE_BUILD_TYPE: Controls optimization and debug symbols
# CMAKE_EXPORT_COMPILE_COMMANDS: Generates compile_commands.json for IDEs
echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

echo ""
echo "Compiling DataNuke..."
# Compile using all available CPU cores for faster builds
# -j$(nproc): Parallel compilation (Linux)
# For macOS, use: make -j$(sysctl -n hw.ncpu)
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "========================================"
echo "  Build Complete!"
echo "========================================"
echo ""
echo "Binary location: ./build/datanuke"
echo ""
echo "Quick Start:"
echo "  ./build/datanuke -h              # Show help"
echo "  ./build/datanuke -e file.dat     # Encrypt file"
echo "  ./build/datanuke -d file.dat     # Delete file securely"
echo "  sudo ./build/datanuke -w /dev/sdX  # Wipe device (DANGEROUS!)"
echo ""
echo "For more information, see README.md"
echo ""
