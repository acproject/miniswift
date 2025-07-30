#!/bin/bash

# Build script for MiniSwift UI Extension
# This script builds the MiniSwift API library and tests the UI extension

set -e

echo "Building MiniSwift UI Extension..."

# Navigate to project root
cd "$(dirname "$0")/.."

# Build the MiniSwift API library
echo "Building MiniSwift API library..."
cd miniswift
swift build
cd ..

# Build the C++ interpreter with UI support
echo "Building C++ interpreter..."
mkdir -p build
cd build
cmake ..
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
cd ..

# Test the UI extension
echo "Testing UI extension..."
./build/miniswift tests/test_ui_extension.swift

echo "UI Extension build and test completed!"