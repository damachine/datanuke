#!/bin/bash

# Quick build script for DataNuke

echo "Building DataNuke..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Run CMake
cmake ..

# Compile
make -j$(nproc)

echo ""
echo "Build complete!"
echo "Run './datanuke -h' for usage information"
